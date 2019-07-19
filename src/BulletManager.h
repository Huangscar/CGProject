#pragma once
#include <vector>

#include "Bullet.h"
#include "EulerCamera.h"
using namespace std;
using namespace GXProject;

class BulletManager
{
public:
	BulletManager();
	~BulletManager();
	void newBullet(glm::vec3 front, glm::vec3 position);
	void render(const glm::vec3 &lightPos, EulerCamera camera);
	void deleteBullet(string bulletId);
	bool Collide(glm::vec3 pos);
private:
	vector<Bullet*> bulletLists;
};
