#include "particlelist.h"
#include "particleconfig.h"
#include "lodepng.h"

#include "GL/glew.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

//#include "util/loadtexture.h"

#include "glm/glm.hpp"
using namespace glm;

GLuint loadPNG(const char* name, bool highQualityMipmaps);

ParticleConfig* muzzleFlash = NULL;
ParticleConfig* smoke = NULL;
ParticleConfig* spark = NULL;
ParticleConfig* impactFlare = NULL;
ParticleConfig* dirtSpray = NULL;

ParticleConfig* trailSmoke = NULL;
ParticleConfig* trailFire = NULL;
ParticleConfig* explodeEmitter = NULL;

void initParticleList()
{
	muzzleFlash = new ParticleConfig();
	muzzleFlash->motionLimits[0] = vec3(0.0);
	muzzleFlash->motionLimits[1] = vec3(0.0);
	muzzleFlash->spinLimits[0] = 0.0;
	muzzleFlash->spinLimits[1] = 0.0;
	muzzleFlash->startSizeLimits[0] = 0.45;
	muzzleFlash->startSizeLimits[1] = 0.45;
	muzzleFlash->endSizeLimits[0] = 0.45;
	muzzleFlash->endSizeLimits[1] = 0.45;
	muzzleFlash->startColorLimits[0] = vec4(1.0, 1.0, 1.0, 1.0);
	muzzleFlash->startColorLimits[1] = vec4(1.0, 1.0, 1.0, 1.0);
	muzzleFlash->endColorLimits[0] = vec4(1.0, 1.0, 1.0, 0.0);
	muzzleFlash->endColorLimits[1] = vec4(1.0, 1.0, 1.0, 0.0);
	muzzleFlash->lifeLimits[0] = 0.05;
	muzzleFlash->lifeLimits[1] = 0.05;
	muzzleFlash->initialGravityLimits[0] = 0.0;
	muzzleFlash->initialGravityLimits[1] = 0.0;
	muzzleFlash->gravityRateLimits[0] = 0.0;
	muzzleFlash->gravityRateLimits[1] = 0.0;
	muzzleFlash->additiveBlending = false;
	muzzleFlash->texture = loadPNG("../res/particle/muzzle-flash.png", true);

	smoke = new ParticleConfig();
	smoke->motionLimits[0] = vec3(-0.008, 0.003, -0.008);
	smoke->motionLimits[1] = vec3(0.008, 0.006, 0.008);
	smoke->spinLimits[0] = -0.0008;
	smoke->spinLimits[1] = 0.0008;
	smoke->startSizeLimits[0] = 0.01;
	smoke->startSizeLimits[1] = 0.01;
	smoke->endSizeLimits[0] = 0.07;
	smoke->endSizeLimits[1] = 0.085;
	smoke->startColorLimits[0] = vec4(0.5, 0.5, 0.5, 1.0);
	smoke->startColorLimits[1] = vec4(0.5, 0.5, 0.5, 1.0);
	smoke->endColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.0);
	smoke->endColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.0);
	smoke->lifeLimits[0] = 0.9;
	smoke->lifeLimits[1] = 1.2;
	smoke->initialGravityLimits[0] = 0.0;
	smoke->initialGravityLimits[1] = 0.0;
	smoke->gravityRateLimits[0] = 0.0;
	smoke->gravityRateLimits[1] = 0.0;
	smoke->additiveBlending = false;
	smoke->texture = loadPNG("../res/particle/smoke.png", true);

	spark = new ParticleConfig();
	spark->motionLimits[0] = vec3(-0.07, 0.0, -0.07);
	spark->motionLimits[1] = vec3(0.07, 0.0, 0.07);
	spark->spinLimits[0] = 0.0;
	spark->spinLimits[1] = 0.0;
	spark->startSizeLimits[0] = 0.05;
	spark->startSizeLimits[1] = 0.3;
	spark->endSizeLimits[0] = 0.0;
	spark->endSizeLimits[1] = 0.0;
	spark->startColorLimits[0] = vec4(1.0, 1.0, 0.5, 1.0);
	spark->startColorLimits[1] = vec4(1.0, 1.0, 1.0, 1.0);
	spark->endColorLimits[0] = vec4(1.0, 1.0, 0.5, 1.0);
	spark->endColorLimits[1] = vec4(1.0, 1.0, 0.5, 1.0);
	spark->lifeLimits[0] = 1.0;
	spark->lifeLimits[1] = 1.0;
	spark->initialGravityLimits[0] = 0.05;
	spark->initialGravityLimits[1] = 0.1;
	spark->gravityRateLimits[0] = -0.3;
	spark->gravityRateLimits[1] = -0.3;
	spark->additiveBlending = false;
	spark->texture = loadPNG("../res/particle/spark.png", false);

	impactFlare = new ParticleConfig();
	impactFlare->motionLimits[0] = vec3(0.0, 0.0, 0.0);
	impactFlare->motionLimits[1] = vec3(0.0, 0.0, 0.0);
	impactFlare->spinLimits[0] = 0.0;
	impactFlare->spinLimits[1] = 0.0;
	impactFlare->startSizeLimits[0] = 1.0;
	impactFlare->startSizeLimits[1] = 1.0;
	impactFlare->endSizeLimits[0] = 0.0;
	impactFlare->endSizeLimits[1] = 0.0;
	impactFlare->startColorLimits[0] = vec4(1.0, 0.9, 0.2, 1.0);
	impactFlare->startColorLimits[1] = vec4(1.0, 0.9, 0.2, 1.0);
	impactFlare->endColorLimits[0] = vec4(1.0, 0.9, 0.2, 0.0);
	impactFlare->endColorLimits[1] = vec4(1.0, 0.9, 0.2, 0.0);
	impactFlare->lifeLimits[0] = 0.1;
	impactFlare->lifeLimits[1] = 0.1;
	impactFlare->initialGravityLimits[0] = 0.0;
	impactFlare->initialGravityLimits[1] = 0.0;
	impactFlare->gravityRateLimits[0] = 0.0;
	impactFlare->gravityRateLimits[1] = 0.0;
	impactFlare->additiveBlending = false;
	impactFlare->texture = loadPNG("../res/particle/flare.png", true);

	dirtSpray = new ParticleConfig();
	dirtSpray->motionLimits[0] = vec3(-0.025, 0.0, -0.025);
	dirtSpray->motionLimits[1] = vec3(0.025, 0.0, 0.025);
	dirtSpray->spinLimits[0] = 0.0;
	dirtSpray->spinLimits[1] = 0.0;
	dirtSpray->startSizeLimits[0] = 0.1;
	dirtSpray->startSizeLimits[1] = 0.2;
	dirtSpray->endSizeLimits[0] = 1.0;
	dirtSpray->endSizeLimits[1] = 1.5;
	dirtSpray->startColorLimits[0] = vec4(0.32, 0.18, 0.04, 2.0) * 0.5f;
	dirtSpray->startColorLimits[1] = vec4(0.37, 0.23, 0.09, 2.0) * 0.5f;
	dirtSpray->endColorLimits[0] = vec4(0.32, 0.18, 0.04, 2.0) * 0.5f;
	dirtSpray->endColorLimits[1] = vec4(0.37, 0.23, 0.09, 2.0) * 0.5f;
	dirtSpray->lifeLimits[0] = 0.5;
	dirtSpray->lifeLimits[1] = 0.8;
	dirtSpray->initialGravityLimits[0] = 0.035;
	dirtSpray->initialGravityLimits[1] = 0.06;
	dirtSpray->gravityRateLimits[0] = -0.3;
	dirtSpray->gravityRateLimits[1] = -0.3;
	dirtSpray->additiveBlending = false;
	dirtSpray->texture = loadPNG("../res/particle/dirt-spray.png", true);

	trailSmoke = new ParticleConfig();
	trailSmoke->motionLimits[0] = vec3(-0.001, 0.001, -0.001);
	trailSmoke->motionLimits[1] = vec3(0.001, 0.003, 0.001);
	trailSmoke->spinLimits[0] = -0.001;
	trailSmoke->spinLimits[1] = 0.001;
	trailSmoke->startSizeLimits[0] = 0.5;
	trailSmoke->startSizeLimits[1] = 0.7;
	trailSmoke->endSizeLimits[0] = 1.5;
	trailSmoke->endSizeLimits[1] = 1.8;
	trailSmoke->startColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.2);
	trailSmoke->startColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.4);
	trailSmoke->endColorLimits[0] = vec4(0.5, 0.5, 0.5, 0.0);
	trailSmoke->endColorLimits[1] = vec4(0.5, 0.5, 0.5, 0.0);
	trailSmoke->lifeLimits[0] = 2.5;
	trailSmoke->lifeLimits[1] = 3.5;
	trailSmoke->initialGravityLimits[0] = 0.0;
	trailSmoke->initialGravityLimits[1] = 0.0;
	trailSmoke->gravityRateLimits[0] = 0.0;
	trailSmoke->gravityRateLimits[1] = 0.0;
	trailSmoke->additiveBlending = false;
	trailSmoke->texture = loadPNG("../res/particle/smoke.png", true);

	trailFire = new ParticleConfig();
	trailFire->motionLimits[0] = vec3(-0.001, 0.001, -0.001);
	trailFire->motionLimits[1] = vec3(0.001, 0.003, 0.001);
	trailFire->spinLimits[0] = 0.0;
	trailFire->spinLimits[1] = 0.0;
	trailFire->randomOrientation = false;
	trailFire->startSizeLimits[0] = 0.5;
	trailFire->startSizeLimits[1] = 2.0;
	trailFire->endSizeLimits[0] = 0.0;
	trailFire->endSizeLimits[1] = 0.0;
	trailFire->startColorLimits[0] = vec4(1.0, 0.8, 0.1, 0.2);
	trailFire->startColorLimits[1] = vec4(1.0, 0.8, 0.3, 0.9);
	trailFire->endColorLimits[0] = vec4(1.0, 0.4, 0.0, 0.0);
	trailFire->endColorLimits[1] = vec4(1.0, 0.7, 0.2, 0.0);
	trailFire->lifeLimits[0] = 0.05;
	trailFire->lifeLimits[1] = 0.4;
	trailFire->initialGravityLimits[0] = 0.0;
	trailFire->initialGravityLimits[1] = 0.0;
	trailFire->gravityRateLimits[0] = 0.3;
	trailFire->gravityRateLimits[1] = 0.3;
	trailFire->additiveBlending = false;
	trailFire->texture = loadPNG("../res/particle/flame.png", true);

	explodeEmitter = new ParticleConfig();
	explodeEmitter->motionLimits[0] = vec3(-0.06, 0.0, -0.06);
	explodeEmitter->motionLimits[1] = vec3(0.06, 0.0, 0.06);
	explodeEmitter->spinLimits[0] = 0.0;
	explodeEmitter->spinLimits[1] = 0.0;
	explodeEmitter->lifeLimits[0] = 0.7;
	explodeEmitter->lifeLimits[1] = 1.2;
	explodeEmitter->initialGravityLimits[0] = 0.04;
	explodeEmitter->initialGravityLimits[1] = 0.07;
	explodeEmitter->gravityRateLimits[0] = -0.25;
	explodeEmitter->gravityRateLimits[1] = -0.25;
	explodeEmitter->childEmissionInterval = 0.01;
	explodeEmitter->children.push_back(trailFire);
	explodeEmitter->children.push_back(trailSmoke);
}

void deinitParticleList()
{
	delete muzzleFlash;
	delete smoke;
	delete spark;
	delete impactFlare;
	delete dirtSpray;
	delete trailSmoke;
	delete trailFire;
	delete explodeEmitter;
}

GLuint loadPNG(const char* name, bool highQualityMipmaps)
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
		std::cout << "loadTexture() could not load " << name << std::endl;
		exit(1);
	}

	return result;
}

