#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 colorIn;
layout (location = 2) in vec2 instancePos;

out vec3 color;
void main()
{
    color = colorIn;
    vec2 finalPos = position.xy + instancePos;
    gl_Position = vec4(finalPos, position.z, 1.0);
}