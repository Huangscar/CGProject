#include "BulletManager.h"

BulletManager::BulletManager()
{
}

BulletManager::~BulletManager()
{
	this->bulletLists.clear();
}


void BulletManager::newBullet(glm::vec3 front, glm::vec3 position) {
	this->bulletLists.push_back(new Bullet(front, position));
}

void BulletManager::render(const glm::vec3 &lightPos, EulerCamera camera) {
	for (int i = 0; i < (int)this->bulletLists.size(); i++) {
        float distance = glm::distance(bulletLists[i]->getPosition(), camera.getPosition());
	    if (distance > 100){
		    deleteBullet(bulletLists[i]->id);
        }else{
		    bulletLists[i]->render(lightPos, camera);
        }
	}
}

void BulletManager::deleteBullet(std::string bulletId) {
	for (auto it = bulletLists.begin(); it != bulletLists.end();) {
		if ((*it)->id == bulletId) {
			it = bulletLists.erase(it);
		}
		else {
			it++;
		}
	}
}

bool BulletManager::Collide(glm::vec3 pos) {
	for (auto it = bulletLists.begin(); it != bulletLists.end();) {
		float x = (*it)->getPosition().x;
		float y = (*it)->getPosition().z;
		if (x >= (pos.x - 0.4) && x <= (pos.x + 0.4) && y >= (pos.z - 0.4) && y <= (pos.z + 0.4)) {
			return true;
		}
		else {
			it++;
		}
	}
	return false;
}
