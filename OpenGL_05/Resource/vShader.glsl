#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fragPos;
out vec4 glNormal;

void main()
{
	fragPos = view * model * position;
	gl_Position = projection * fragPos;
	glNormal = view * model * vec4(normal, 0.0f);
}