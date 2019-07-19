#ifndef _FIRE_SHADER_H_
#define _FIRE_SHADER_H_

#include "ShaderProgram.h"

class FireShader : public ShaderProgram {
public:
	virtual void LoadProgram();
	FireShader();
	~FireShader();
};

#endif