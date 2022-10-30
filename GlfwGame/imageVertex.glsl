#version 330 core
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texture_position;

out vec2 f_tex_coord;

void main()
{
	gl_Position = vec4(a_position, 0.5, 1.0);
	f_tex_coord = a_texture_position;
}