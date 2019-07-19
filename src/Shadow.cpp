#include "Shadow.h"
#include "filesystem.h"
#include "stb_image.h"

Shadow::Shadow() {
	
	glEnable(GL_DEPTH_TEST);


	

	

	
}

void Shadow::init() {
	depthShader = std::make_shared<Shader>("../src/Shaders/simpleDepthShader.vs", "../src/Shaders/simpleDepthShader.fs");
	shadowShader = std::make_shared<Shader>("../src/Shaders/shadow_mapping.vs", "../src/Shaders/shadow_mapping.fs");
	
	shadowShader->bind();
	shadowShader->setUniform("diffuseTexture", 0);
	shadowShader->setUniform("shadowMap", 1);

	//lightPos = glm::vec3(17.9, 2.0, 18.7);
	//lightPos = glm::vec3(18.9907, 3.76, 18.6);
	//lightPos = glm::vec3(17.7, 1.2, 18.7); // last try
	//lightPos = glm::vec3(0.5, 1.76, 0.5);
	//lightPos = glm::vec3(18.0, 1.2, 18.0);

	/////////////try
	//lightPos = glm::vec3(18, 2.0, 19);
	//lightPos = glm::vec3(19, 1.0, 19);
	// ok1 lightPos = glm::vec3(17, 2.3, 17.9);
	//lightPos = glm::vec3(17.2, 2.2, 18.2); // ok2
	//lightPos = glm::vec3(17.2, 3.7, 18.2);
	lightPos = glm::vec3(20, 3.7, 18.2);



	///////////////////////////////////
	//lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);
	//(17.7, 1.0, 18.7)
	//testCube._cube->position = vec3(17.7, 1.2, 18.7);
	

	/*float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};*/

	/*float planeVertices[] = {
		// positions            // normals         // texcoords
		 125.0f, 1.1f,  125.0f,  0.0f, 1.0f, 0.0f,  125.0f,  0.0f,
		-125.0f, 1.1f,  125.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-125.0f, 1.1f, -125.0f,  0.0f, 1.0f, 0.0f,   0.0f, 125.0f,

		 125.0f, 1.1f,  125.0f,  0.0f, 1.0f, 0.0f,  125.0f,  0.0f,
		-125.0f, 1.1f, -125.0f,  0.0f, 1.0f, 0.0f,   0.0f, 125.0f,
		 125.0f, 1.1f, -125.0f,  0.0f, 1.0f, 0.0f,  125.0f, 125.0f
	};*/

	float planeVertices[] = {
		// positions            // normals         // texcoords
		 125.0f, 1.01f,  125.0f,  0.0f, 1.0f, 0.0f,  125.0f,  0.0f,
		-125.0f, 1.01f,  125.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-125.0f, 1.01f, -125.0f,  0.0f, 1.0f, 0.0f,   0.0f, 125.0f,

		 125.0f, 1.01f,  125.0f,  0.0f, 1.0f, 0.0f,  125.0f,  0.0f,
		-125.0f, 1.01f, -125.0f,  0.0f, 1.0f, 0.0f,   0.0f, 125.0f,
		 125.0f, 1.01f, -125.0f,  0.0f, 1.0f, 0.0f,  125.0f, 125.0f
	};

	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);


	// load textures
	// -------------
	woodTexture = loadTexture("../res/Textures/wood.png");
	//woodTexture = loadTexture(FileSystem::getPath("res/Ground/wall.png").c_str());

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
// render the cube
void Shadow::renderCube() {
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// render the scene
void Shadow::renderScene(glm::mat4 floor_modelmatrix, glm::mat4 cube_modelmatrix, ShaderPtr shader) {
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	//shader->setUniform("model", floor_modelmatrix);
	shader->setUniform("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// cubes
	/*model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader->setUniform("model", model*cube_modelmatrix);
	renderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader->setUniform("model", model);
	renderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader->setUniform("model", model);
	renderCube();*/
	//lightPos = glm::vec3(18.0, 1.2, 18.7);
	//glm::mat4 tmpModel = getModel(lightPos);
	glm::vec3 rot = glm::vec3(0, 0, 0);
	glm::vec3 scl = glm::vec3(0.1, 0.1, 0.1);
	glm::mat4 tmpModel = getModel(lightPos, rot, scl);

	// cubes
	shader->setUniform("model", cube_modelmatrix);
	renderCube();
	shader->setUniform("model", tmpModel);
	//renderCube();
	
}

glm::mat4 Shadow::getModel(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1)) {
	glm::mat4 tm = glm::translate(glm::mat4(1), position);
	glm::mat4 rotx = glm::rotate(glm::mat4(1), rotation.x, glm::vec3(1, 0, 0));
	glm::mat4 roty = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
	glm::mat4 rotz = glm::rotate(glm::mat4(1), rotation.z, glm::vec3(0, 0, 1));
	glm::mat4 grot = rotx * roty * rotz;
	glm::mat4 sclMat = glm::scale(glm::mat4(1), scale);

	return tm * grot* sclMat;
}


void Shadow::render(glm::mat4 floor_modelmatrix, glm::mat4 cube_modelmatrix, EulerCamera camera) {
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	//EulerCamera lightCamera; //cube = glm::vec3(17.7, 1.0, 18.7);
	//lightCamera.Reset(lightPos, glm::vec3(17.7, 1.0, 18.7), glm::vec3(0, 1, 0));
	//lightPos = glm::vec3(18.0, 1.2, 18.7);
	//lightCamera.setPosition(lightPos); //last try
	//lightCamera.setPosition(glm::vec3(0.5, 1.76, 0.5));
	//lightProjection = lightCamera.getProjectionMatrix();
	//lightView = lightCamera.getViewMatrix();
	//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)1024 / (GLfloat)1024, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(17.7, 1.0, 18.7), glm::vec3(0.0, 1.0, 0.0));
	
	//glm::mat4 tmpModel = getModel(lightPos);
	glm::vec3 rot = glm::vec3(0, 0, 0);
	glm::vec3 scl = glm::vec3(0.1, 0.1, 0.1);
	glm::mat4 modellight = getModel(lightPos, rot, scl);
	lightSpaceMatrix = lightProjection * lightView;
	//lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	depthShader->bind();
	depthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glm::mat4 model = glm::mat4(1.0f);
	renderScene(floor_modelmatrix, cube_modelmatrix, depthShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. render scene as normal using the generated depth/shadow map  
	// --------------------------------------------------------------
	glViewport(0, 0, 800, 600);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadowShader->bind();
	glm::mat4 projection = camera.getProjectionMatrix();
	glm::mat4 view = camera.getViewMatrix();
	shadowShader->setUniform("projection", projection);
	shadowShader->setUniform("view", view);

	shadowShader->setUniform("viewPos", camera.getPosition());
	shadowShader->setUniform("lightPos", lightPos);
	shadowShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
	
	//shader.setVec3("viewPos", camera.Position);
	//shader.setVec3("lightPos", lightPos);
	//shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	renderScene(floor_modelmatrix, cube_modelmatrix, shadowShader);

}

// shaderPtr
ShaderPtr depthShader;
ShaderPtr shadowShader;

// using the getModelMatrix to calculate the model matrix
void setModelMatrix(glm::mat4 modelMatrix){}

// using the camera.getViewMatrix * camera.getProjectionMatrix to get the VP and pass in
void setVPMatrix(glm::mat4 VP){}

unsigned int Shadow::loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

