#include "Text.h"
#include "filesystem.h"
#include "EulerCamera.h"
#include <vector>
using namespace GXProject;

Text::Text() {
	


}

void Text::init() {
	// Shader
	//textShader = std::make_shared<Shader>("../src/Shaders/center.vertexshader", "../src/Shaders/center.fragmentshader");

	textShader = std::make_shared<Shader>("../src/Shaders/font.vs", "../src/Shaders/font.fs");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader->bind();
	//textShader.setMat4("projection", projection); 
	textShader->setUniform("projection", projection);

	// 加载字体的面
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: COULD NOT INIT FREETYPE LIBRARY" << std::endl;
	}

	FT_Face face;
	if (FT_New_Face(ft, "../res/arial.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: FAILED TO LOAD FONT" << std::endl;
	}
	
	// 定义文字大小
	// 宽度值设置为0表示我们要从字体面给出的高度动态计算出字形的宽度
	FT_Set_Pixel_Sizes(face, 0, 48);




	

	// 只生成表示128个ASCII字符的字符表
	// 为每一个字符储存纹理和一些度量值
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 禁用byte-alignment限制（？？？） 使用一个字节来表示纹理颜色
	for (GLubyte c = 0; c < 128; c++) {
		// 加载字符的字形
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYPE: FAILED TO LOAD GLYPH" << std::endl;
			continue;
		}

		// 生成字形纹理
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// 设置纹理选项
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 将字符存储字符表中备用
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};

		Characters.insert(std::pair<GLchar, Character>(c, character));

	}

	glBindTexture(GL_TEXTURE_2D, 0);

	// 清理FreeType的资源
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// 创建VBO、VAO渲染方块
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 每个2D方块有6个顶点 每个顶点又是一个4维向量（一个纹理坐标+一个顶点坐标）
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 正交投影矩阵
	//glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

}

void Text::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	//cout << "hhh" << endl;
	//  激活合适的渲染状态
	textShader->bind();
	textShader->setUniform("textColor", color);

	//glUniform3f(glGetUniformLocation(textShader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// 对文本中的所有字符迭代
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		//cout << "xpos: " << xpos << endl;
		//cout << "ypos: " << ypos << endl;
		//cout << "h: " << h << endl;
		//cout << "w: " << w << endl;

		// 当前字符的VBO
		GLfloat vertices[6][4] = {
			{xpos, ypos + h, 0.0, 0.0},
			{xpos, ypos, 0.0, 1.0},
			{xpos + w, ypos, 1.0, 1.0},

			{xpos, ypos + h, 0.0, 0.0},
			{xpos + w, ypos, 1.0, 1.0},
			{xpos + w, ypos + h, 1.0, 0.0}
		};

		// test projection vertices
		vector<glm::vec4> allVertices;
		glm::vec4 one_test = glm::vec4(xpos, ypos + h, 0.0, 1.0);
		glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
		glm::vec4 new_pos = projection * one_test;
		//cout << "new pos: " << new_pos[0] << ", " << new_pos[1] << ", " << new_pos[2] << ", " << new_pos[3] << endl;


		// 在方块上绘制字形纹理
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// 更改当前字符的VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// 绘制方块
		//glEnable(GL_BLEND);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDisable(GL_BLEND);
		// 更新位置到下一个字形的原点
		// scale是什么
		x += (ch.Advance >> 6) * scale;


	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
