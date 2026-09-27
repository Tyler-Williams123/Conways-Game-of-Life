#version 430 core
#define SIDE_SIZE 128
#define SIDE_LENGTH (2.0 / SIDE_SIZE)

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 cursorPos;

out vec3 localPos;
void main()
{
    localPos = position;

    vec2 finalPos = position.xy + vec2((cursorPos.x + 0.5) * SIDE_LENGTH - 1, (cursorPos.y + 0.5) * SIDE_LENGTH - 1);
    gl_Position = vec4(finalPos, position.z, 1.0);
}