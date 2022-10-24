#version 330 core

in vec2 position;

out vec4 FragColor;

void main()
{
    FragColor = vec4( position.yxy,1.0f);
}