#version 330 core
layout (location = 0) in vec2 a_Position;

uniform vec2 u_position;
uniform float u_scale;

void main()
{
	gl_Position = vec4(a_Position * u_scale + u_position, 0.0, 1.0);
}