#ifndef _SHADOW_H_
#define _SHADOW_H_
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "EulerCamera.h"

#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
using namespace std;
using namespace GXProject;

class Shadow {
public:

	// init
	Shadow();
	// render the cube
	void renderCube();

	// render the scene
	void renderScene(glm::mat4 floor_modelmatrix, glm::mat4 cube_modelmatrix, ShaderPtr shader);

	// all
	void render(glm::mat4 floor_modelmatrix, glm::mat4 cube_modelmatrix, EulerCamera camera);

	// shaderPtr
	ShaderPtr depthShader;
	ShaderPtr shadowShader;

	// using the getModelMatrix to calculate the model matrix
	void setModelMatrix(glm::mat4 modelMatrix);

	// using the camera.getViewMatrix * camera.getProjectionMatrix to get the VP and pass in
	void setVPMatrix(glm::mat4 VP);

	unsigned int loadTexture(char const* path);

	void init();

	glm::mat4 getModel(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

private:
	unsigned int planeVAO;
	unsigned int planeVBO;
	unsigned int cubeVAO;
	unsigned int cubeVBO;
	unsigned int depthMapFBO;
	unsigned int depthMap;
	unsigned int woodTexture;
	glm::vec3 lightPos;


};
#endif