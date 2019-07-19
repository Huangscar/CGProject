#include <stb_image.h>
#include"Game.h"
#include "Geometry.h"
#include "Application.h"
#include "Text.h"
#include <glm/glm.hpp>
#include <iostream>
#include "Helper.h"
#include "particlelist.h"
using namespace GXProject;
using namespace std;
using namespace glm;

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

const int nx = 256;
const int ny = 256;

float dt = 0.02f;
float iterations = 5;
float vorticity = 10.0f;

glm::vec3 position_, rotation_, scale_;

Fire myFire(nx, ny, dt, iterations, vorticity);
Shadow myShadow;

MeshPtr _mesh;

Text myText;
bool isEnd(vec3 pos, vec3 cubePos, float size) {
	float x1 = cubePos.x - size;
	float x2 = cubePos.x + size;
	float y1 = cubePos.z - size;
	float y2 = cubePos.z + size;

	if (pos.x >= x1 && pos.x <= x2 && pos.z >= y1 && pos.z <= y2)
		return true;
	else
		return false;
}

Game::Game(){
	const int MAX_PARTICLES = 5000;


	auto windowpos = Application::get()->getWindowSize();
	Application::get()->setMousePosition(windowpos.x/2,windowpos.y/2);
	_turnOnMouseCamera = false;
	
	/// fire (fluid simulation)
	myFire.init();
	myShadow.init();

	// text
	myText.init();


	_genericShader = std::make_shared<Shader>("../src/Shaders/Model.vertexshader", "../src/Shaders/Model.fragmentshader");


	auto _skyboxShader = std::make_shared<Shader>("../src/Shaders/Skybox.vertexshader", "../src/Shaders/Skybox.fragmentshader");

	auto meshTest = Helper::loadMesh("../res/cube/cube.obj");

	sky.setShader(_skyboxShader);

	setupCenter();
	setupGun();

	// set up bullets
	bulletManager = new BulletManager();
	
	//setupGround(10,10,nullptr,nullptr);
	_ground = new Ground(20,20);
	_walls = Ground::loadFile("../res/defaultMaze.txt");

	meshTest->setShader(_genericShader);
	//finalCube._cube->position = vec3(17.7, 0.7, 18.7);
	finalCube._cube->position = vec3(17.7, 1.7, 18.5);
	finalCube._cube->scale = vec3(0.5, 0.5, 0.5);
	finalCube._cube->mesh = meshTest;
	
	
	//testCube._cube->position = vec3(18.9907, 3.76, 20.6);
	//finalCube._cube->position = vec3(17.7, 1.7, 18.7);
	testCube._cube->position = vec3(15.7, 1.2, 18.7);
	testCube._cube->scale = vec3(0.1, 0.1, 0.1);
	testCube._cube->mesh = meshTest;
	
	playerLight.position = camera.getPosition();
	Monsterlight.intensity = 1;
	Monsterlight.color = vec3(1,0,0);
	Monsterlight.position = vec3(0,3,0);
	playerLight.intensity = 1;
	playerLight.color = vec3(1,1,1);

	FireLight.position = vec3(17.9, 2.0, 18.7);
	FireLight.intensity = 1;
	FireLight.color = vec3(1,1, 0);

	//monsterPosition = vec3(2.25,1.6,15.4);
	monsterPosition = vec3(3.9, 1.5, 3.4);
	monsterPosTarget = monsterPosition;
	monsterRotate = vec3(-90,0,180);
	model_m = glm::translate(monsterPosition) * glm::rotate(-90.0f,vec3(1.0f,0.0f,0.0f)) *glm::rotate(180.0f,vec3(0.0f,0.0f,1.0f)) * glm::scale(vec3(0.009f,0.009f,0.009f));

	//load model file..
	mymodel.LoadModel("../res/0-MD2model/s0/s0/tris.MD2");
 

	//set curent animation to play..
	model_anim_state = mymodel.StartAnimation(animType_t::CROUCH_WALK);

	//load animation shader.
	animatedModelShader.LoadProgram();

	_debugMode = false;
	//camera.Reset(vec3(0),vec3(-0.2,4.8,15.6),vec3(0,1,0));
	//camera.setPosition(vec3(16.5284, 2.76, 17.7648)); -> shadow debug 专用
	//camera.setPosition(vec3(17.5407, 1.76, 17.7923));
	camera.setPosition(vec3(0.5, 1.76, 0.5)); // -> 起点

	_simpleDepthShader = std::make_shared<Shader>("../src/Shaders/simpleDepthShader.vs", "../src/Shaders/simpleDepthShader.fs");

	// particle
	initParticleList();

	//snow = new Snow::Snow();

	particles = new ParticleManager(MAX_PARTICLES, _walls->getMaze(), _walls->getX(), _walls->getY());
}

Game::~Game(){
	delete _ground;
	delete _walls;
}

//here for test purposes
void Game::setupGround(int x, int y, ShaderPtr shdr, MeshPtr msh)
{
	auto meshTest = Helper::loadMesh("../res/cube/cube.obj");
	auto _genericShader = std::make_shared<Shader>("../src/Shaders/Model.vertexshader", "../src/Shaders/Model.fragmentshader");
	meshTest->setShader(_genericShader);
	string id = "ground_";
	for(int i=0;i<x;i++)
	{
		for(int j=0;j<y;j++)
		{
			auto c = std::make_shared<GameObject>(meshTest);
			c->position.x = i;
			c->position.z = j;
			c->scale = glm::vec3(0.5);
			scene[id+std::to_string(i)+"_"+std::to_string(j)]=c;
		}
	}
}

void Game::setupShaderUniforms(ShaderPtr shdr)
{
		shdr->setUniform("TxSampler",0);
		shdr->setUniform("NrmSampler",1);

		shdr->setUniform("ViewMatrix",camera.getViewMatrix());
		//do the lights here
		shdr->setUniform("eyePos",camera.getPosition());

		shdr->setUniform("playerLightPosition",playerLight.position);
		shdr->setUniform("playerLightColor",playerLight.color);
		shdr->setUniform("playerLightIntensity",playerLight.intensity);

		shdr->setUniform("monsterLightPosition",Monsterlight.position);
		shdr->setUniform("monsterLightColor",Monsterlight.color);
		shdr->setUniform("monsterLightIntensity",Monsterlight.intensity);

		shdr->setUniform("fireLightPosition", FireLight.position);
		shdr->setUniform("fireLightColor", FireLight.color);
		shdr->setUniform("fireLightIntensity", FireLight.intensity);

}

void Game::update(double delta){

	//cout<<"update"<<endl;
	cameraMouseMovement(0,0);
	//moving the light with the camera.

	Monsterlight.position = monsterPosition;
	playerLight.position = camera.getPosition();
	//cout<<playerLight.position.x<<" | "<<playerLight.position.y<<" | "<< playerLight.position.z<<endl;
	//cout<<_walls->Collide(playerLight.position)<<endl;
	
	mymodel.UpdateAnimation(&model_anim_state,delta / 1000);
	MonsterAI();
	auto mmx = glm::rotate(monsterRotate.x,vec3(1,0,0));
	auto mmy = glm::rotate(monsterRotate.y,vec3(0,1,0));
	auto mmz = glm::rotate(monsterRotate.z,vec3(0,0,1));
	auto mmr = mmx*mmy*mmz;
	model_m = glm::translate(monsterPosition) * mmr * glm::scale(vec3(0.009f,0.009f,0.009f));

	glm::mat4 projection(1.0f);
	glm::mat4 model(1.0f);
	glm::mat4 view = camera.getViewMatrix();
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600, 0.1f, 2000.f);
	//floor.render(model,view,projection);
	//snow.Render(delta, model, view, projection);

	//particle
	particles->recycle();
	particles->update(dt);


}

void Game::render(){
	glEnable(GL_DEPTH_TEST);


	//glm::vec3(17.9, 2.0, 18.7)
	glm::vec3 new_pos = vec3(17.7, 1.1, 18.7);
	//position_ = vec3(2.10359, 1.76, 0.562899);
	glm::vec3 new_rot = glm::vec3(0);
	glm::vec3 new_scl = glm::vec3(0.2, 0.2, 0.2);
	glm::mat4 newCubeMatrix = getModel(new_pos, new_rot, new_scl);

	new_pos = vec3(17.7, 0.0, 18.7);
	//position_ = vec3(2.10359, 1.76, 0.562899);
	new_rot = glm::vec3(0);
	new_scl = glm::vec3(1, 1, 1);
	glm::mat4 newFloorMatrix = getModel(new_pos, new_rot, new_scl);

	glEnable(GL_BLEND);
	myShadow.render(newFloorMatrix, newCubeMatrix, camera);
	glDisable(GL_BLEND);

	
	
	//cout<<"render"<<endl;
	glm::mat4 VP = camera.getProjectionMatrix()*camera.getViewMatrix();
	sky.draw(camera.getProjectionMatrix()*mat4(mat3(camera.getViewMatrix())));

	

	for (auto obj : scene){
		auto objShader = obj.second->mesh->getMaterial()->shader;
		objShader->bind();
		objShader->setUniform("TxSampler",0);
		objShader->setUniform("NrmSampler",1);

		objShader->setUniform("ViewMatrix",camera.getViewMatrix());
		//do the lights here
		objShader->setUniform("eyePos",camera.getPosition());

		objShader->setUniform("playerLightPosition",playerLight.position);
		objShader->setUniform("playerLightColor",playerLight.color);
		objShader->setUniform("playerLightIntensity",playerLight.intensity);

		objShader->setUniform("monsterLightPosition",Monsterlight.position);
		objShader->setUniform("monsterLightColor",Monsterlight.color);
		objShader->setUniform("monsterLightIntensity",Monsterlight.intensity);

		objShader->setUniform("fireLightPosition", FireLight.position);
		objShader->setUniform("fireLightColor", FireLight.color);
		objShader->setUniform("fireLightIntensity", FireLight.intensity);

		//probably we can do it once for all objects but who cares it's a college project anyway.
		//end of the lights
		obj.second->draw(VP);
	}

	//_ground->draw(VP);
	

	_walls->draw(VP);
	_ground->draw(VP);

	
	//finalCube._cube->draw(VP);
	//testCube._cube->draw(VP);

	// render bullets
	bulletManager->render(playerLight.position, camera);
	
	//
	animatedModelShader.UseProgram();
	animatedModelShader.BindVPMatrix(&VP[0][0]); //need vp matrix to render model in..

	animatedModelShader.BindModelMatrix(&model_m[0][0]); //need model matrix itself..

	mymodel.RenderModel(&model_anim_state,&animatedModelShader);

	

	// Fire position (fluid simulation)
	//position_ = vec3(17.9, 2.0, 18.7);
	//position_ = vec3(16.5284, 2.76, 17.7648);
	position_ = vec3(17.9907, 2.0, 18.5);
	//position_ = vec3(2.10359, 1.76, 0.562899);
	rotation_ = glm::vec3(0);
	scale_ = glm::vec3(0.5, 0.5, 0.5);
	// scale -> 2.5
	// pos -> position_ = vec3(18.9907, 3.76, 18.6);

	glm::mat4 fmodel = getModel(position_, rotation_, scale_);
	//cout << "fmodel " << fmodel[0][0] << endl;

	_fireShader->bind();
	_fireShader->setUniform("VP", VP);
	_fireShader->setUniform("ModelMatrix", fmodel);
	
	// render fire
	glEnable(GL_BLEND);
	glBindVertexArray(_VAO);
	myFire.on_frame(_fireShader, camera);
	glDisable(GL_BLEND);

	// text
	// render text
	glEnable(GL_BLEND);
	myText.RenderText("Maze shooting game", 60.0f, 500.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	glDisable(GL_BLEND);


	// render aiming center 
	_centerShader->bind();
	//  bind Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture);

	// draw aiming cente
	glEnable(GL_BLEND);
	glBindVertexArray(_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);

	// render gun
	//renderGun(camera);

	// particle
	/*position_ = vec3(17.9907, 2.0, 18.5);
	rotation_ = glm::vec3(0);
	scale_ = glm::vec3(0.5, 0.5, 0.5);
	glm::mat4 fmodel = getModel(position_, rotation_, scale_);
	

	_fireShader->bind();
	_fireShader->setUniform("VP", VP);
	_fireShader->setUniform("ModelMatrix", fmodel);
	glm::mat4 VP = camera.getProjectionMatrix() * camera.getViewMatrix();*/
	glm::vec3 sd = camera.getSide();
	particles->render(camera.getProjectionMatrix(), camera.getViewMatrix(), camera.getSide(), camera.getUp());

	cout << "==== side ====" << endl;
	cout << sd[0] << " " << sd[1] << " " << sd[2] << endl;

	//snow->Render(0.002f, mat4(1.0f), camera.getViewMatrix(), camera.getProjectionMatrix());


	


}

void Game::addParticle(ParticleConfig* config, vec3 pos, float lifeFactor)
{

	//vec3(model * vec4(aPos, 1.0));
	//glm::vec3 new_pos = vec3(17.7, 1.1, 18.7);
	//position_ = vec3(2.10359, 1.76, 0.562899);
	
	//glm::vec3 rot = glm::vec3(0);
	//glm::vec3 scl = glm::vec3(1.0, 1.0, 1.0);
	//glm::mat4 fmodel = getModel(pos, rot, scl);
	//pos = vec3(fmodel * vec4(pos, 1.0));
	particles->add(config, pos, lifeFactor);
}

void Game::renderAll() {
	

}
void Game::MonsterAI(){
	int rnd = rand()%4;
	vec3 bpos = monsterPosition;
	if(monsterPosTarget == monsterPosition){
		if(rnd == 1){
			//std::cout << "????" << std::endl;

			monsterPosTarget.x += 0.5;
			monsterDirection = Direction::NORTH;
			monsterRotate.z = 0;
		}else if(rnd==2){
			monsterPosTarget.x -= 0.5;
			monsterDirection = Direction::SOUTH;
			monsterRotate.z = 180;
		}else if(rnd==3){
			monsterPosTarget.z += 0.5;
			monsterDirection = Direction::EAST;
			monsterRotate.z = 270;
		}else if(rnd==0){
			monsterPosTarget.z -= 0.5;
			monsterDirection = Direction::WEST;
			monsterRotate.z = 90;
		}
		if(_walls->Collide(monsterPosTarget))
			monsterPosTarget = monsterPosition;
		//bpos = monsterPosTarget;
	}else{
		if(monsterDirection == Direction::NORTH){
			bpos.x += 0.05;
		}else if(monsterDirection == Direction::SOUTH){
			bpos.x -= 0.05;
		}else if(monsterDirection == Direction::EAST){
			bpos.z += 0.05;
		}else if(monsterDirection == Direction::WEST){
			bpos.z -= 0.05;
		}
	}

	monsterPosition = bpos;
	if(_walls->Collide(bpos)){
		monsterPosTarget = monsterPosition;

		if(monsterDirection == Direction::NORTH){
			monsterPosTarget.x -= 0.5;
		}else if(monsterDirection == Direction::SOUTH){
			monsterPosTarget.x += 0.5;
		}else if(monsterDirection == Direction::EAST){
			monsterPosTarget.z -= 0.5;
		}else if(monsterDirection == Direction::WEST){
			monsterPosTarget.z += 0.5;
		}
		monsterPosition = monsterPosTarget;
	}else{
		if (isEnd(camera.getPosition(), monsterPosition, 0.4f)) {
			Application::get()->endGame();
		}
		if (bulletManager->Collide(monsterPosition)) {
			// add a quick, bright flash of impact
			// 
			// playerLight.position
			// addParticle(impactFlare, impactPoint);
			addParticle(impactFlare, monsterPosition);
			for (int i = 0; i < 10; i++)
			{
				addParticle(smoke, monsterPosition);
			}
			
			//addParticle(impactFlare, playerLight.position);

			monsterPosition = vec3(100, 1.7, 100);
		}
	}

	//cout<<"Direction = "<<monsterDirection<<endl;
}
void Game::handleKeyboard(GLFWwindow* window,int key, int scancode,int action,int mods){
	//std::cout<<"Key Event"<<std::endl;
	vec3 bpos;
	switch (key)
	{
	case GLFW_KEY_W:
		bpos = camera.getPosition();
		camera.Walk(0.1);
		if(_walls->Collide(camera.getPosition())&& !_debugMode)
			camera.Walk(-0.1);
		if (isEnd(camera.getPosition(), finalCube._cube->position, 0.2f) || isEnd(camera.getPosition(), monsterPosition, 0.4f)) {
			camera.Walk(-0.1);
			Application::get()->endGame();
		}
		if (camera.getPosition().z >= 19.0f) {
			camera.Walk(-0.1);
		}
		break;
	case GLFW_KEY_S:
		bpos = camera.getPosition();
		camera.Walk(-0.1);
		if(_walls->Collide(camera.getPosition()) && !_debugMode)
			camera.Walk(0.1);
		if (isEnd(camera.getPosition(), finalCube._cube->position, 0.2f) || isEnd(camera.getPosition(), monsterPosition, 0.4f)) {
			camera.Walk(0.1);
			Application::get()->endGame();
		}
		break;
	case GLFW_KEY_A:
		bpos = camera.getPosition();
		camera.Strafe(-0.1);
		if(_walls->Collide(camera.getPosition())&& !_debugMode)
			camera.Strafe(0.1);
		if (isEnd(camera.getPosition(), finalCube._cube->position, 0.2f) || isEnd(camera.getPosition(), monsterPosition, 0.4f)) {
			camera.Strafe(0.1);
			Application::get()->endGame();
		}
		break;
	case GLFW_KEY_D:
		bpos = camera.getPosition();
		camera.Strafe(0.1);
		if(_walls->Collide(camera.getPosition())&& !_debugMode)
			camera.Strafe(-0.1);
		if (isEnd(camera.getPosition(), finalCube._cube->position, 0.2f) || isEnd(camera.getPosition(), monsterPosition, 0.4f)) {
			camera.Strafe(-0.1);
			Application::get()->endGame();
		}
			
		break;
	case GLFW_KEY_ESCAPE:
		Application::get()->shutdown();
		break;
	case GLFW_KEY_UP:
		Monsterlight.position.y++;
		break;
	case GLFW_KEY_DOWN:
		Monsterlight.position.y--;
		break;
	case GLFW_KEY_HOME:
		if(action == GLFW_PRESS)
			_debugMode = !_debugMode;
		break;
	}
	if(!_debugMode){
		auto y = camera.getPosition();
		y.y = 1.76;
		camera.setPosition(y);
	}
	camera.updateViewMatrix();
	
}

void Game::handleMouseButtons(GLFWwindow* window,int button,int action,int mods){
	if (action == GLFW_PRESS) {
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout<<"GLFW_MOUSE_BUTTON_LEFT Event"<<std::endl;
			bulletManager->newBullet(-camera.getLookDirection(), playerLight.position);
			std::cout << "Player light pos: " << playerLight.position[0] << " " << playerLight.position[1] << " " << playerLight.position[2] << std::endl;
			//std::cout << "camera look direction: " << camera.getLookDirection()[0] << " " << camera.getLookDirection()[1] << " " << camera.getLookDirection()[2] << std::endl;
			
			//addParticle(impactFlare, glm::vec3(0.5, 1.76, 0.5));
			
			//cout << "PlayerLight position " << playerLight.position[0] << " " << playerLight.position[1] << " " << playerLight.position[2] << std::endl;

			/*for (int i = 0; i < 10; i++)
			{
				addParticle(smoke, monsterPosition);
			}*/
			break;
		default:
			break;
		}
	}
}

void Game::handleMouseMotion(GLFWwindow* window,int x, int y){
	mousePosition = vec2(x,y);
}

void Game::cameraMouseMovement(int x, int y){
	auto windowSize = Application::get()->getWindowSize();
	if (mousePosition.x != windowSize.x/2
		|| mousePosition.y != windowSize.y/2)
	{
		//cout<<"MOUSE MOVED"<<endl;
		double mouseSpeed = 0.005; //it is just there to speed up or slow down the movements.
		double movedDistanceX;
		double movedDistanceY;

		//  the distance (old position - new position)
		// in other words:  how far is the mouse from the center of the window ? The bigger this value, the more we want to turn.
		// note that the old position (x_old, y_old) is fixed in this application (x_old = WindoSizeWidth/2, y_old = WindowSizeHeight/2)
		movedDistanceX = double(windowSize.x/2 - mousePosition.x)*mouseSpeed;
		movedDistanceY = double(windowSize.y/2 - mousePosition.y)*mouseSpeed;

		// Pass the two distances to the Renderer (our drawings)
		
			camera.Yaw(movedDistanceX);
			camera.Pitch(movedDistanceY);
			camera.updateViewMatrix();
		

		//Force the new position of the mouse to be in the middle of the window
		mousePosition.x = windowSize.x/2;
		mousePosition.y = windowSize.y/2;
		Application::get()->setMousePosition(mousePosition.x,mousePosition.y);
	}
}


void Game::setupCenter(){
	/*float center_vertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        -0.1f,  0.1f,  0.9f,  0.0f,  0.0f,
        -0.1f, -0.1f,  0.9f,  0.0f,  1.0f,
         0.1f, -0.1f,  0.9f,  1.0f,  1.0f,

        -0.1f,  0.1f,  0.9f,  0.0f,  0.0f,
         0.1f, -0.1f,  0.9f,  1.0f,  1.0f,
         0.1f,  0.1f,  0.9f,  1.0f,  0.0f
    };*/

	float center_vertices[] = {
		// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
		360.0f,  330.0f,  0.0f,  0.0f,  0.0f,
		360.0f, 270.0f,  0.0f,  0.0f,  1.0f,
		440.0f, 270.0f,  0.0f,  1.0f,  1.0f,

		360.0f,  330.0f,  0.0f,  0.0f,  0.0f,
		440.0f,  270.0f,  0.0f,  1.0f,  1.0f,
		440.0f,  330.0f,  0.0f,  1.0f,  0.0f
	};

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(center_vertices), center_vertices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // texture coord attribute
	glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load and create a texture 
    // -------------------------
	glGenTextures(1, &_texture);

    int width, height, nrComponents;
    unsigned char *data = stbi_load("../res/textures/center.png", &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << std::endl;
        stbi_image_free(data);
    }

	// set shader of aming center 
	_centerShader = std::make_shared<Shader>("../src/Shaders/center.vertexshader", "../src/Shaders/center.fragmentshader");
	_centerShader->bind();
	_centerShader->setUniform("centerTxc", 0);
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	_centerShader->setUniform("projection", projection);

	_fireShader = std::make_shared<Shader>("../src/Shader/fluid_m.vs", "../src/Shader/fluid.fs");
	
}


// get the model matrix
glm::mat4 Game::getModel(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1)) {
	glm::mat4 tm = glm::translate(glm::mat4(1), position);
	glm::mat4 rotx = glm::rotate(glm::mat4(1), rotation.x, glm::vec3(1, 0, 0));
	glm::mat4 roty = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
	glm::mat4 rotz = glm::rotate(glm::mat4(1), rotation.z, glm::vec3(0, 0, 1));
	glm::mat4 grot = rotx * roty * rotz;
	glm::mat4 sclMat = glm::scale(glm::mat4(1), scale);

	return tm * grot* sclMat;
}

void Game::setupShadow() {
	glGenFramebuffers(1, &_depthMapFBO);
	glGenTextures(1, &_depthMap);
	glBindTexture(GL_TEXTURE_2D, _depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}



GLuint Game::loadPNG(const char *name, bool highQualityMipmaps = false)
{
    unsigned int width;
    unsigned int height;
    unsigned char *data;
    unsigned char *temp;
    unsigned int i;
    GLuint result = 0;

	// use a simple stand-alone library to load our PNGs
	lodepng_decode32_file(&data, &width, &height, name);

	// make sure the load was successful
	if(data)
	{
		// the pixel data is flipped vertically, so we need to flip it back with an in-place reversal
		temp = new unsigned char[width * 4];
		for(i = 0; i < height / 2; i ++)
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
		if(highQualityMipmaps)
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
		//cout << "hhhh7777" << endl;
	}
	else
	{
		cerr << "loadTexture() could not load " << name << endl;
		exit(1);
	}

    return result;
}

void Game::setupGun(){
	
	GLMmodel *geometry;
    // attempt to read the file; glmReadObj() will just quit if we can't
    geometry = glmReadOBJ((char*)"../res/gun/gun.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		// build our buffer objects and then fill them with the geometry data we loaded
		glGenVertexArrays(1, &gunVAO);
		glBindVertexArray(gunVAO);
		glGenBuffers(3, gunVBO);
		glmBuildVBO(geometry, &numGunVertices, &gunVAO, gunVBO);
	}

	_gunShader = std::make_shared<Shader>("../src/Shaders/gun.vs", "../src/Shaders/gun.fs");
	_gunShader -> bind();
	_gunShader -> setUniform("a_Vertex", 0);
	_gunShader -> setUniform("a_Normal", 1);
	_gunShader -> setUniform("a_TexCoord", 2);
	_gunShader -> setUniform("u_DiffuseMap", 0);
	_gunShader -> setUniform("u_NormalMap", 1);
	_gunShader -> setUniform("u_SpecularMap", 2);
	_gunShader -> setUniform("u_EmissionMap", 3);
	_gunShader -> setUniform("u_Sun", playerLight.position);
	_gunShader -> setUniform("u_MaterialDiffuse", vec3(1.0, 0.95, 0.85));
	_gunShader -> setUniform("u_MaterialSpecular", vec3(1.0, 0.95, 0.85));
	_gunShader -> setUniform("u_SpecularIntensity", 8.0f);
	_gunShader -> setUniform("u_SpecularHardness", 2.0f);
	_gunShader -> setUniform("u_NormalMapStrength", 2.5f);

	gunDiffuseMap = loadPNG("../res/gun/gun-diffuse-map.png");
	gunNormalMap = loadPNG("../res/gun/gun-normal-map.png");
	gunSpecularMap = loadPNG("../res/gun/gun-specular-map.png");
	gunEmissionMap = loadPNG("../res/gun/gun-emission-map.png");
}

void Game::renderGun(EulerCamera camera){
	
	_gunShader -> bind();

	glm::mat4 gunMat;	
	glm::mat4 normalMat;
	glm::mat4 viewLocalMat = camera.getViewMatrix();
	// gunMat = glm::translate(gunMat, playerLight.position - camera.getLookDirection() * 2.0f);


	// orient the gun in the same direction as the camera (with the recoil and reloading offsets applied)
	gunMat[0] = glm::vec4(camera.getSide(), 0.0);
	gunMat[1] = glm::vec4(camera.getUp(), 0.0);
	gunMat[2] = glm::vec4(camera.getLookDirection(), 0.0);
	gunMat[3] = glm::vec4(playerLight.position - camera.getUp() * 0.2f - camera.getLookDirection() * 0.8f, 1.0);
	gunMat = glm::scale(gunMat, glm::vec3(0.5f, 0.5f, 0.5f));

	// compute our normal matrix for lighting
	normalMat = glm::inverseTranspose(glm::mat3(gunMat));

	_gunShader -> setUniform("u_Projection", camera.getProjectionMatrix());
	_gunShader -> setUniform("u_View", viewLocalMat);
	_gunShader -> setUniform("u_Model", gunMat);
	_gunShader -> setUniform("u_Normal", normalMat);

	// we use diffuse, normal, specular, and emission texture maps when rendering for a really nice effect
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gunDiffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gunNormalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gunSpecularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gunEmissionMap);

	// we don't want this gun transparent
	glDisable(GL_BLEND);

	// finally, render it
	glBindVertexArray(gunVAO);
	glDrawArrays(GL_TRIANGLES, 0, numGunVertices);
}