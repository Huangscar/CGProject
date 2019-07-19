#version 330 core

layout (location = 0) in vec2 a_Vertex;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform mat4 u_View;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Vertex, 0.0, 1.0);
}
