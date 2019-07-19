#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//#include "shader_m.h"
//#include "camera.h"

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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

#define DEFAULT_NX 256
#define DEFAULT_NY 256





// util function
template <typename T>
struct Grid {
	T* values;
	int nx, ny;

	Grid(int nx, int ny) : nx(nx), ny(ny) {
		values = new T[nx * ny];
	}

	Grid(const Grid&) = delete; // 禁用
	Grid& operator = (const Grid&) = delete; // 禁用

	~Grid() {
		delete[] values;
	}

	void swap(Grid& other) {
		std::swap(values, other.values);
		std::swap(nx, other.nx);
		std::swap(ny, other.ny);
	}

	const T* data() const {
		return values;
	}

	int idx(int x, int y) const {
		x = (x + nx) % nx; // ?????
		y = (y + ny) % ny; // ?????

		return x + y * nx;
	}

	T & operator()(int x, int y) {
		return values[idx(x, y)];
	}

	const T& operator()(int x, int y) const {
		return values[idx(x, y)];
	}
};






class Fire {
public:
	Fire(int nx = 256, int ny = 256, float dt = 0.02f, float iterations = 5, float vorticity = 10.0f);
	// void addFire(); // add_density
	void add_density(int px, int py, int r = 10, float value = 0.5f);
	float curl(int x, int y);
	void advect_velocity();
	void advect_density();
	void project_velocity();
	void on_frame(ShaderPtr fluidShader, EulerCamera& camera);
	//void on_frame(Shader fluidShader, Camera& camera);
	void init();
	void fluid_simulation_step();
	void vorticity_confinement();
	void on_move(int x, int y);
	void on_move2(GLFWwindow* window, double xpos, double ypos);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);

	int nx;
	int ny;

private:

	float dt;
	float iterations;
	float vorticity;



	GLuint texture;

	glm::vec2 mouse;

	unsigned int fireVAO, fireVBO;

};


