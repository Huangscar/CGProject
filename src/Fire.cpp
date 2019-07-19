#include "Fire.h"

#define FOR_EACH_CELL for(int y = 0; y < Fire::ny; y++) for (int x = 0; x < Fire::nx; x++)


#ifdef _WIN32
#include <windows.h>

double sec() {
	LARGE_INTEGER frequency, t;
	QueryPerformanceCounter(&t);
	QueryPerformanceFrequency(&frequency);
	return t.QuadPart / (double)frequency.QuadPart;
}
#else
#include <time.h>

double sec() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + 1e-9 * t.tv_nsec;
}
#endif



Grid<glm::vec2> old_velocity(DEFAULT_NX, DEFAULT_NY);
Grid<glm::vec2> new_velocity(DEFAULT_NX, DEFAULT_NY);
Grid<float> old_density(DEFAULT_NX, DEFAULT_NY);
Grid<float> new_density(DEFAULT_NX, DEFAULT_NY);

Grid<uint32_t> pixels(DEFAULT_NX, DEFAULT_NY);

//==============utils========================

template <typename T, typename U>
T lerp(const T& a, const T& b, const U& u) {
	return (U(1) - u) * a + u * b;
}

template <typename T>
const T& clamp(const T & x, const T & a, const T & b) {
	return
		x < a ? a :
		x > b ? b :
		x;
}

template <typename T, typename U>
T smoothstep(const T & a, const T & b, const U & u) {
	T t = clamp((u - a) / (b - a), U(0), U(1));
	return t * t * (U(3) - U(2) * t);
}

float randf(float a, float b) {
	float u = rand() * (1.0 / RAND_MAX);
	return lerp(a, b, u); // 线性插值??? 范围
}

template <typename T>
T interpolate(const Grid<T> & grid, glm::vec2 p) {
	int ix = floorf(p.x);
	int iy = floorf(p.y);
	float ux = p.x - ix;
	float uy = p.y - iy;
	return lerp(
		lerp(grid(ix + 0, iy + 0), grid(ix + 1, iy + 0), ux),
		lerp(grid(ix + 0, iy + 1), grid(ix + 1, iy + 1), ux),
		uy
	);
}

uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
	r = clamp(r, 0u, 255u);
	g = clamp(g, 0u, 255u);
	b = clamp(b, 0u, 255u);
	a = clamp(a, 0u, 255u);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t rgba(float r, float g, float b, float a) {
	return rgba32(r * 256, g * 256, b * 256, a * 256);
}

float length(glm::vec2 a) {
	return sqrtf(dot(a, a));
}



//===========================================
Fire::Fire(int nx, int ny, float dt, float iterations, float vorticity) :nx(nx), ny(ny), dt(dt), iterations(iterations), vorticity(vorticity) {

	Grid<glm::vec2> old_velocity_tmp(nx, ny);
	Grid<glm::vec2> new_velocity_tmp(nx, ny);
	Grid<float> old_density_tmp(nx, ny);
	Grid<float> new_density_tmp(nx, ny);
	Grid<uint32_t> pixels_tmp(nx, ny);

	old_velocity.swap(old_velocity_tmp);
	new_velocity.swap(new_velocity_tmp);
	old_density.swap(old_density_tmp);
	new_density.swap(new_density_tmp);
	pixels.swap(pixels_tmp);
}

void Fire::add_density(int px, int py, int r, float value) {
	//std::cout << "in add density" << std::endl;
	for (int y = -r; y <= r; y++) {
		for (int x = -r; x <= r; x++) {
			float d = sqrtf(x * x + y * y); // 计算斜边
			float u = smoothstep(float(r), 0.0f, d);  // Hermite interpolation
			//cout << "u " << u << endl;
			old_density(px + x, py + y) += u * value;
		}
	}
}


float Fire::curl(int x, int y) {
	return
		old_velocity(x, y + 1).x - old_velocity(x, y - 1).x +
		old_velocity(x - 1, y).y - old_velocity(x + 1, y).y;
}


void Fire::advect_velocity() {
	FOR_EACH_CELL{
		glm::vec2 pos = glm::vec2(x, y) - dt * old_velocity(x, y);
		new_velocity(x, y) = interpolate(old_velocity, pos);
	}

	old_velocity.swap(new_velocity);
}

void Fire::advect_density() {
	FOR_EACH_CELL{
		glm::vec2 pos = glm::vec2(x, y) - dt * old_velocity(x, y);
		new_density(x, y) = interpolate(old_density, pos);
	}
	old_density.swap(new_density);
}

void Fire::project_velocity() {
	Grid<float> p(nx, ny);
	Grid<float> p2(nx, ny);
	Grid<float> div(nx, ny);

	FOR_EACH_CELL{
		float dx = old_velocity(x + 1, y + 0).x - old_velocity(x - 1, y + 0).x;
		float dy = old_velocity(x + 0, y + 1).y - old_velocity(x + 0, y - 1).y;
		div(x, y) = dx + dy;
		p(x, y) = 0.0f;
	}

	for (int k = 0; k < iterations; k++) {
		FOR_EACH_CELL{
			float sum = -div(x, y)
				+ p(x + 1, y + 0)
				+ p(x - 1, y + 0)
				+ p(x + 0, y + 1)
				+ p(x + 0, y - 1);
			p2(x, y) = 0.25f * sum;
		}

		p.swap(p2);
	}

	FOR_EACH_CELL{
		old_velocity(x, y).x -= 0.5f * (p(x + 1, y + 0) - p(x - 1, y + 0));
		old_velocity(x, y).y -= 0.5f * (p(x + 0, y + 1) - p(x + 0, y - 1));

	}
}

//void Fire::on_frame(Shader fluidShader, Camera& camera) {
void Fire::on_frame(ShaderPtr fluidShader, EulerCamera & camera) {

	fluid_simulation_step();

	double t = sec();
	//cout << "t " << t << endl;
	// density field to pixels
	FOR_EACH_CELL{
		float rf = old_density(x, y);
		float f = log2f(rf * 0.25f + 1.0f);
		float f3 = f * f * f;
		float r = 1.5f * f;
		float g = 1.5f * f3;
		float b = f3 * f3;

		if (rf > 0.5f) {
			pixels(x, y) = rgba(r, g, b, 1.0);
			//cout << "rf " << rf << endl;
		}
		else {
			pixels(x, y) = rgba(r, g, b, 0.0);
		}



	}



	double dt = sec() - t;
	//printf("!!!! %f\n", dt * 1000);




	// ...something needs to be added....


	//upload pixels to texture


	//................
	fluidShader->bind();
	//glm::mat4 model = glm::mat4(1.0f);
	//glm::mat4 view = camera.getViewMatrix();


	//glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT), 0.1f, 100.0f);
	// origin
	//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	// using new camera
	
	//glm::mat4 projection = camera.getProjectionMatrix();

	// origin
	/*fluidShader.setMat4("model", model);
	fluidShader.setMat4("view", view);
	fluidShader.setMat4("projection", projection);*/

	//fluidShader->setUniform("model", model);
	//fluidShader->setUniform("view", view);
	//fluidShader->setUniform("projection", projection);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	//glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	// origin
	//glUniform1i(glGetUniformLocation(fluidShader.ID, "ourTexture"), 0);
	// use new shader class
	fluidShader->setUniform("ourTexture", 0);


	glBindVertexArray(fireVAO);
	//cout << "draw" << endl;
	glDrawArrays(GL_TRIANGLES, 0, 6);
#if 0
	// To make video from frames:
	// ffmpeg -i frame_%d.ppm video.mp4
	static int frame = 0;
	char path[256];
	snprintf(path, sizeof(path), "frames/frame_%d.ppm", frame);
	screenshot(path);
	puts(path);
	frame++;
	if (frame >= 1024) {
		exit(0);
	}
#endif
}

void Fire::init() {
	FOR_EACH_CELL{
		old_density(x, y) = 0.0f;
		old_velocity(x, y) = glm::vec2(0.0f, 0.0f);
	}


	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	FOR_EACH_CELL{

		pixels(x, y) = rgba(0.0f, 0.0f, 0.0f, 0.0);

	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());


	//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	//float fluidVertices[] = {
	//	// positions     // texture
	//	-1.0f, -1.0f,    0.0f, 0.0f,
	//	1.0f, -1.0f,     1.0f, 0.0f,
	//	1.0f, 1.0f,      1.0f, 1.0f,

	//	1.0f, 1.0f,     1.0f, 1.0f,
	//	-1.0f, 1.0f,      0.0f, 1.0f,
	//	-1.0f, -1.0f,     0.0f, 0.0f
	//};

	float fluidVertices[] = {
		// positions     // texture
		-1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,    1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,     1.0f, 1.0f,

		1.0f, 1.0f,  0.0f,   1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,     0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,    0.0f, 0.0f
	};



	glGenVertexArrays(1, &fireVAO);
	glGenBuffers(1, &fireVBO);
	glBindVertexArray(fireVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fireVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fluidVertices), &fluidVertices, GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Fire::fluid_simulation_step() {
	//std::cout << "in!" << std::endl;
	FOR_EACH_CELL{
		if (x > nx * 0.5f) continue; // 

		float r = 10.0f;
		old_velocity(x, y).x += randf(-r, +r);
		old_velocity(x, y).y += randf(-r, +r);

	}

		// Dense regions rise up
		FOR_EACH_CELL{
			old_velocity(x, y).y += (old_density(x, y) * 20.0f - 5.0f) * dt; //????
	}

		//add_density(mouse.x, mouse.y, 10, 0.5f);

	// fast movement is dampened???
		FOR_EACH_CELL{
			old_density(x, y) *= 0.99f;
	}

	add_density(nx * 0.25f, 30);
	//add_density(nx * 0.75f, 30);

	double t[10];

	t[0] = sec();
	//cout << "t[0]: " << t[0] << endl;
	vorticity_confinement(); ///???????
	t[1] = sec(); //?????

	advect_velocity();

	t[2] = sec();
	project_velocity();
	t[3] = sec();

	advect_density();
	t[4] = sec();

	// zero out stuff at bottom
	FOR_EACH_CELL{
		if (y < 10) {
			old_density(x, y) = 0.0f;
			old_velocity(x, y) = glm::vec2(0.0f, 0.0f);
		}
	}

	for (int i = 0; i < 4; i++) {
		t[i] = (t[i + 1] - t[i]) * 1000;
	}

	char title[256];
	//snprintf(title, sizeof(title), "%f %f %f %f\n", t[0], t[1], t[2], t[3]);
	//cout << "title: " << title << endl;
	// 把title设置为窗口标题
	//glfwSetWindowTitle(window, title);

}

void Fire::vorticity_confinement() {
	//cout << "in vorticity" << endl;
	Grid<float> abs_curl(nx, ny);

	FOR_EACH_CELL{
		abs_curl(x, y) = fabsf(curl(x, y));
	}

		FOR_EACH_CELL{
			glm::vec2 direction;
			direction.x = abs_curl(x + 0, y - 1) - abs_curl(x + 0, y + 1);
			direction.y = abs_curl(x + 1, y + 0) - abs_curl(x - 1, y + 0);

			direction = vorticity / (length(direction) + 1e-5f) * direction;

			if (x < nx / 2) direction *= 0.0f;

			new_velocity(x, y) = old_velocity(x, y) + dt * curl(x, y) * direction;


	}

	old_velocity.swap(new_velocity);
}

void Fire::on_move(int x, int y) {
	y = SCR_HEIGHT - 1 - y;
	mouse = glm::vec2(x * 1.0f * nx / SCR_WIDTH, y * 1.0f * ny / SCR_HEIGHT);
}

void Fire::on_move2(GLFWwindow * window, double xpos, double ypos) {
	int y = (int)SCR_HEIGHT - 1 - ypos;
	int x = (int)xpos;
	//std::cout << "x: " << x << " y: " << y << std::endl;
	mouse = glm::vec2(x, y);
}

void Fire::on_mouse_button(GLFWwindow * window, int button, int action, int mods) {

	// ...
	double posX, posY;
	glfwGetCursorPos(window, &posX, &posY);

	on_move((int)posX, (int)posY);

	// 单击鼠标左键
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		add_density(mouse.x, mouse.y, 10, 300.0f);
		//std::cout << posX << ", " << posY << std::endl;
	}
}