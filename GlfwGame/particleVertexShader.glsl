#version 330 core

layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec2 a_Offset;

out vec2 position;

void main()
{
    vec2 pos = a_Pos * (gl_InstanceID/20.0) + a_Offset;
    gl_Position = vec4(pos*0.1, 0.0, 1.0);
    position = gl_Position.xy;
}