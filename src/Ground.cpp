#include "Ground.h"
#include "Helper.h"
#include "Application.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace GXProject;
using namespace std;

Ground::Ground(int _x, int _y):GameObject(NULL)
{
	x = _x;
	y = _y;
	maze = NULL;
	auto meshTest = Helper::loadMesh("../res/Ground/cube.obj");
	auto _genericShader = std::make_shared<Shader>("../src/Shaders/Model.vertexshader", "../src/Shaders/Model.fragmentshader");
	meshTest->setShader(_genericShader);
	_cube = std::make_shared<GameObject>(meshTest);
	//_cube->scale = glm::vec3(0.25);

	for(int i=0;i<x;i++)
	{
		for(int j=0;j<y;j++)
		{
			_offsets.push_back(glm::vec3(i,0,j));
		}
	}
}

Ground::~Ground()
{
	if(maze != NULL){
		for(int i=0;i<y;i++){
			delete[] maze[i];
		}
	}
	delete[] maze;
}

Ground* Ground::loadFile(std::string path)
{
	Ground* res = NULL;

	ifstream file(path,std::ios::in);
	if(file.is_open())
	{
		string line;
		int x,y;
		getline(file,line);
		x = atoi(line.c_str());
		getline(file,line);
		y = atoi(line.c_str());
		res = new Ground(x,y);

		res->maze = new char*[x];
		for(int i=0;i<y;i++)
			res->maze[i] = new char[y];

		auto meshTest = Helper::loadMesh("../res/cube/cube.obj");
		auto _genericShader = std::make_shared<Shader>("../src/Shaders/Model.vertexshader", "../src/Shaders/Model.fragmentshader");
		meshTest->setShader(_genericShader);
		res->_cube = std::make_shared<GameObject>(meshTest);
		int xx = 0;
		res->_offsets.clear();
		while(getline(file,line))
		{
			for(int i=0;i<res->y;i++)
			{
				res->maze[xx][i]=line[i];
				if(line[i]=='#')
					res->_offsets.push_back(glm::vec3(xx,1,i));
				else
					res->_offsets.push_back(glm::vec3(0));
			}
			xx++;
		}
		file.close();
	}else
	{
		cout<<"can't load map from file = "<<path<<endl;
		return NULL;
	}

	return res;
}

void Ground::draw(glm::mat4 vp,int is_Instancing)
{
		auto shdr = _cube->mesh->getMaterial()->shader;
		shdr->bind();
		Application::getGame()->setupShaderUniforms(shdr);

		shdr->setUniform("isInstancing",1);

		for(int i=0;i<_offsets.size();i++)
		{
			shdr->setUniform("offsets["+to_string(i)+"]",_offsets[i]);
		}

		_cube->draw(vp,x*y);
}

bool Ground::Collide(vec3 pos){
	vec3 pos1 = pos + vec3(0.8,0,0.8);
	vec3 pos2 = pos + vec3(1.2, 0, 1.2);
	vec3 pos3 = pos + vec3(0.8, 0, 1.2);
	vec3 pos4 = pos + vec3(1.2, 0, 0.8);
	if(maze != NULL){
		int x1 = pos1.x;
		int y1 = pos1.z;
		int x2 = pos2.x;
		int y2 = pos2.z;
		int x3 = pos3.x;
		int y3 = pos3.z;
		int x4 = pos4.x;
		int y4 = pos4.z;

		if(x1>=this->x || x1<0)
			return false;

		if(y1>=this->y || y1<0)
			return false;

		if(maze[x1][y1] == '#' || maze[x2][y2] == '#' || maze[x3][y3] == '#' || maze[x4][y4] == '#')
			return true;
		else
			return false;
	}else
		return false;
}

std::vector<std::string> Ground::getMaze() {
	vector<string> result;
	for (int i = 0; i < x; i++) {
		string lineStr = "";
		for (int j = 0; j < y; j++) {
			lineStr += maze[i][j];
		}
		result.push_back(lineStr);
	}
	return result;
}

int Ground::getY() {
	return y;
}

int Ground::getX() {
	return x;
}