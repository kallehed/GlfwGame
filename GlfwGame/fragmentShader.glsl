#version 330 core

out vec4 FragColor;

in vec3 position;

uniform float time;

void main()
{
    FragColor = vec4(position.xy, abs(sin(time)), 1.f);
}