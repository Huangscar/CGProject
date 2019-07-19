#ifndef _BULLET_H_
#define _BULLET_H_
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
#include <time.h>
#include <algorithm>
using namespace std;
using namespace GXProject;


class Bullet{
public:
	static float velocity;
	static string genRandomString();
	string id;
	Bullet(glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f));
	~Bullet();
    glm::vec3 getPosition() const{return position;};
	void render(const glm::vec3 &lightPos, EulerCamera camera);
private:
	glm::vec3 position;
	glm::vec3 front;
	ShaderPtr shader;
	void InitialData();
	GLuint VAO, VBO;
};
#endif