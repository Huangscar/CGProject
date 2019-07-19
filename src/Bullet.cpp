#include "Bullet.h"

float Bullet::velocity = 2.0f;

Bullet::Bullet(glm::vec3 front, glm::vec3 pos)
{
	this->position = pos;
	this->front = front;
	this->shader = std::make_shared<Shader>("../src/Shaders/bullet_light.vs", "../src/Shaders/bullet_light.fs");
	this->id = Bullet::genRandomString();
	InitialData();
}


// Bullet::~Bullet()
// {
// 	if (shader) {
// 		delete shader;
// 		shader = nullptr;
// 	}
// }

void Bullet::InitialData()
{
	GLfloat vertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

}

void Bullet::render(const glm::vec3 &lightPos, EulerCamera camera) {
	// don't forget to enable shader before setting uniforms
	this->shader->bind();

	// view/projection transformations
	glm::mat4 projection = camera.getProjectionMatrix();
	glm::mat4 view = camera.getViewMatrix();
	this->shader->setUniform("projection", projection);
	this->shader->setUniform("view", view);

	glm::mat4 model = glm::mat4(1.0f);
	this->position = this->position + this->front * Bullet::velocity;
	model = glm::translate(model, this->position);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	// 子弹按当前yaw, pitch旋转，使得射出子弹时只看到它的尾部
	model = glm::rotate(model, glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(camera.getPitch()), glm::vec3(0.0f, 0.0f, 1.0f));
	this->shader->setUniform("model", model);

	// 光照设置
	float ambientStrength = 0.5f;
	float diffuseStrength = 1.0f;
	float specularStrength = 0.8f;
	int shininess = 64;
	this->shader->setUniform("ambientStrength", ambientStrength);
	this->shader->setUniform("diffuseStrength", diffuseStrength);
	this->shader->setUniform("specularStrength", specularStrength);
	this->shader->setUniform("shininess", shininess);
	this->shader->setUniform("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	this->shader->setUniform("lightPos", lightPos);
	this->shader->setUniform("viewPos", camera.getPosition());

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

std::string Bullet::genRandomString() {
	srand(time(NULL));
	std::string res = "";
	for (int i = 0; i < 20; i++) {
		switch ((rand() % 3))
		{
		case 1:
			res += 'A' + rand() % 26;
			break;
		case 2:
			res = 'a' + rand() % 26;
			break;
		case 3:
			res = '0' + rand() % 10;
			break;
		default:
			break;
		}
	}
	return res;
}