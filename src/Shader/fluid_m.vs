#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 VP;
uniform mat4 ModelMatrix;
uniform mat4 projection;

void main(){
	gl_Position = VP * ModelMatrix * vec4(position, 1.0f);
	TexCoord = texCoord;
}