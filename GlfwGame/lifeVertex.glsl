#version 330 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in float a_color;

in int gl_InstanceID;

uniform vec2 u_offset;
uniform float u_quad_length;
uniform int u_m_SIZE;

out float f_color;

void main()
{
	vec2 pos_instance = vec2(gl_InstanceID%u_m_SIZE, gl_InstanceID/u_m_SIZE);
	vec2 pos = u_offset + u_quad_length * (a_Position + pos_instance);
	gl_Position = vec4(pos, 0.0, 1.0);
	f_color = a_color;
}