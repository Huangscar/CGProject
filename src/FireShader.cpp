#include "FireShader.h"


FireShader::FireShader(void)
{
}


FireShader::~FireShader(void)
{
}

void FireShader::LoadProgram()
{
	programID = LoadShaders("../src/Shader/fluid_m.vs", "../src/Shader/fluid.fs");

	modelMatrixID = glGetUniformLocation(programID, "ModelMatrix");

	vpMatrixID = glGetUniformLocation(programID, "VP");

	
}

