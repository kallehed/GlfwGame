#version 330 core

out vec4 FragColor;

in float f_color;

void main()
{
	FragColor = vec4(vec3(1.0)*f_color, 1.0);
}