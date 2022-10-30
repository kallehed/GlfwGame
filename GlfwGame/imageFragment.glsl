#version 330 core

in vec2 f_tex_coord;

out vec4 FragmentColor;

uniform sampler2D u_texture;

void main()
{
	FragmentColor = texture(u_texture, f_tex_coord);
}