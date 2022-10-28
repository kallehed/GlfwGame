#version 330 core
layout (location = 0) in vec2 a_Position;

in int gl_InstanceID;

uniform vec2 u_offset;
uniform float u_quad_length;
uniform float u_color[2500];

out float f_color;

void main()
{
	vec2 pos = u_offset + a_Position * u_quad_length + vec2((gl_InstanceID % 50) * u_quad_length, (gl_InstanceID/50)*u_quad_length);
	gl_Position = vec4(pos, 0.0, 1.0);
	f_color = u_color[gl_InstanceID];
}