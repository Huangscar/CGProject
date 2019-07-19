#include "Application.h"
#include <exception>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"				// for lookAt() and perspective()
#include "glm/gtx/rotate_vector.hpp"
using namespace glm;
using namespace GXProject;
using namespace std;

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

ApplicationPtr Application::_instance = nullptr;

Application::Application(int GLMAJOR /* = 2 */, int GLMINOR /* = 1 */){
	_versionMajor = GLMAJOR;
	_versionMinor = GLMINOR;
	_time = 0;
	_shutdown = false;
	_endGame = false;
}
Application::~Application(){
	if(_window)
		glfwDestroyWindow(_window);

	glfwTerminate();
}

ApplicationPtr Application::get(int GLMajor,int GLMinor){
	if(_instance == nullptr){
		_instance = ApplicationPtr(new Application(GLMajor,GLMinor));
	}
	return _instance;
}

GamePtr Application::getGame(){
	return Application::get()->_game;
}

glm::vec2 Application::getMousePosition(){
	return _mousePos;
}

void Application::setMousePosition(float x, float y){
	glfwSetCursorPos(_window,x,y);
	_mousePos = glm::vec2(x,y);
}

bool Application::init(unsigned int width /* = 800 */, unsigned int height /* = 600 */, std::string title /* = "Window" */){
	_width = width;
	_height = height;
	_title = title;
	try{
		initWindow();
		return true;
	}catch(exception e){
		cerr<<e.what()<<endl;
		return false;
	}
}

void Application::initWindow(){
	if(!glfwInit())
		throw exception("Cannot initialise GLFW");

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _versionMajor); //OpenGL version 3.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _versionMinor); // 3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE); //If requesting an OpenGL version below 3.2, GLFW_OPENGL_ANY_PROFILE

	_window = glfwCreateWindow(_width,_height,_title.c_str(),NULL,NULL);
	if(_window == nullptr)
		throw exception("Cannot create GLFW Window");

	glfwMakeContextCurrent(_window);

	if(glewInit() != GLEW_OK)
		throw exception("Cannot initialise GLEW");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Application::start(GamePtr game){
	_game = game;
	glfwSetWindowSizeCallback(_window,&Application::windowResized);
	glfwSetKeyCallback(_window,&Application::KeyboardKeys);
	glfwSetMouseButtonCallback(_window,&Application::MouseButtons);
	glfwSetCursorPosCallback(_window,&Application::mouseMotion);
	startLoop();
}

GLuint Application::loadPNG(const char* name, bool highQualityMipmaps = true)
{
	unsigned int width;
	unsigned int height;
	unsigned char* data;
	unsigned char* temp;
	unsigned int i;
	GLuint result = 0;

	// use a simple stand-alone library to load our PNGs
	lodepng_decode32_file(&data, &width, &height, name);

	// make sure the load was successful
	if (data)
	{
		// the pixel data is flipped vertically, so we need to flip it back with an in-place reversal
		temp = new unsigned char[width * 4];
		for (i = 0; i < height / 2; i++)
		{
			memcpy(temp, &data[i * width * 4], (width * 4));								// copy row into temp array
			memcpy(&data[i * width * 4], &data[(height - i - 1) * width * 4], (width * 4));	// copy other side of array into this row
			memcpy(&data[(height - i - 1) * width * 4], temp, (width * 4));					// copy temp into other side of array
		}

		// we can generate a texture object since we had a successful load
		glGenTextures(1, &result);
		glBindTexture(GL_TEXTURE_2D, result);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// texture UVs should not clamp
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// generate high-quality mipmaps for this texture?
		if (highQualityMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		// release the memory used to perform the loading
		delete[] data;
		delete[] temp;
	}
	else
	{
		cerr << "loadTexture() could not load " << name << endl;
		exit(1);
	}

	return result;
}


void Application::showLoadingScreen()
{
	// near and far clip range for ortho perspective; good defaults are -1.0 and 1.0
	const float ORTHO_NEAR_RANGE = -1.0;
	const float ORTHO_FAR_RANGE = 1.0;

	// how we want to the logo to appear
	vec4 LOGO_COLOR(1.0, 1.0, 1.0, 0.1);
	const vec3 LOGO_SIZE(256.0, 256.0, 0.0);

	ApplicationPtr app = Application::get();

	const vec3 LOGO_POSITION(app->_width / 2.0f, app->_height / 2.0f, 0.0);

	// how we want the loading message to appear
	vec4 LOADING_COLOR(1.0, 1.0, 1.0, 0.7);
	const vec3 LOADING_SIZE(128.0, 128.0, 0.0);
	const vec2 LOADING_MARGIN(100, 50);
	const vec3 LOADING_POSITION(app->_width - LOADING_MARGIN.x, app->_height, 0.0);

	// this handles our rendering efficiently
	PlaneRenderer* planes = PlaneRenderer::getInstance();

	// load the "no drones" texture and the "loading" texture
	GLuint logo = loadPNG("../res/page/no-drones.png");
	GLuint loading = loadPNG("../res/page/loading.png");

	// compute the plane renderer's viewing params
	mat4 orthoProjection = glm::ortho(0.0f, (float)app->_width, 0.0f, (float)app->_height, ORTHO_NEAR_RANGE, ORTHO_FAR_RANGE);
	mat4 orthoView = mat4(1.0);
	mat4 model;

	// dark grey background
	glClearColor(0.09, 0.09, 0.09, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the plane renderer's viewing parameters
	planes->bindShader();
	planes->setProjectionMatrix(orthoProjection);
	planes->setViewMatrix(orthoView);

	// draw "no drones" logo
	glBindTexture(GL_TEXTURE_2D, logo);
	model = mat4(1.0);
	model = translate(model, LOGO_POSITION);
	model = scale(model, LOGO_SIZE);
	planes->setColor(LOGO_COLOR);
	planes->setModelMatrix(model);
	planes->render();

	// draw loading message
	glBindTexture(GL_TEXTURE_2D, loading);
	model = mat4(1.0);
	model = translate(model, LOADING_POSITION);
	model = scale(model, LOADING_SIZE);
	planes->setColor(LOADING_COLOR);
	planes->setModelMatrix(model);
	planes->render();

	// swap buffers so we can see everything
	glfwPollEvents();
	glfwSwapBuffers(_window);
}
void Application::startLoop(){
	// Add startup page here
	showLoadingScreen();
	_sleep(1000);

	while(!glfwWindowShouldClose(_window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		update();
		render();
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSwapBuffers(_window);
		glfwPollEvents();
		//if shutdown then break the loop
		if(_shutdown)
			break;
		if (_endGame)
			break;
	}
}

void Application::windowResized(GLFWwindow* window, int width, int height){
	ApplicationPtr app = Application::get();
	app->_width = width;
	app->_height = height;
	glViewport(0,0,width,height);
}

void Application::KeyboardKeys(GLFWwindow* window,int key, int scancode,int action,int mods){
	_instance->_game->handleKeyboard(window,key,scancode,action,mods);
}
void Application::MouseButtons(GLFWwindow* window,int button,int action,int mods){
	_instance->_game->handleMouseButtons(window,button,action,mods);
}
void Application::mouseMotion(GLFWwindow* window,double x, double y){
	_instance->_game->handleMouseMotion(window,x,y);
}

void Application::update(){
	double currentTime = glfwGetTime();
	double deltaTime = (currentTime - _time)*1000;
	_time = currentTime;

	_game->update(deltaTime);
}

void Application::render(){
	_game->render();
}

glm::vec2 Application::getWindowSize(){
	return glm::vec2(_width,_height);
}

void Application::shutdown(){
	_shutdown = true;
}

void Application::endGame() {
	_endGame = true;
}