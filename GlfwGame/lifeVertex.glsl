#version 330 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in float a_color;

in int gl_InstanceID;

uniform vec2 u_offset;
uniform float u_quad_length;

out float f_color;

const int m_SIZE = 1000;

void main()
{
	//vec2 pos = u_offset + a_Position * u_quad_length + vec2((gl_InstanceID % m_SIZE) * u_quad_length, (gl_InstanceID/m_SIZE)*u_quad_length);
	vec2 pos = (a_Position + vec2((gl_InstanceID % m_SIZE), (gl_InstanceID/m_SIZE)) + u_offset) * u_quad_length;
	gl_Position = vec4(pos, 0.0, 1.0);
	f_color = a_color;
}