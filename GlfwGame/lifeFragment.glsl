#version 330 core

out vec4 FragColor;

uniform float u_color;

void main()
{
	FragColor = vec4(vec3(1.0)*u_color, 1.0);
}