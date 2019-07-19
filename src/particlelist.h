#pragma once

class ParticleConfig;

extern "C" ParticleConfig* muzzleFlash;
extern "C" ParticleConfig* smoke;
extern "C" ParticleConfig* spark;
extern "C" ParticleConfig* impactFlare;
extern "C" ParticleConfig* dirtSpray;

extern "C" ParticleConfig* trailSmoke;
extern "C" ParticleConfig* trailFire;
extern "C" ParticleConfig* explodeEmitter;

void initParticleList();
void deinitParticleList();



