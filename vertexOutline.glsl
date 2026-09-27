#version 430 core
#define SIDE_SIZE 128
#define SIDE_LENGTH (2.0 / SIDE_SIZE)

layout (location = 0) in vec3 position;
layout (binding = 1) buffer readBoard{
    uint board[];
};

out vec3 localPos;
int index = gl_InstanceID;
void main()
{
    localPos = position;
    int x = index % 128;
    int y = index / 128;

    vec2 finalPos = position.xy + vec2((x + 0.5) * SIDE_LENGTH - 1, (y + 0.5) * SIDE_LENGTH - 1);
    gl_Position = vec4(finalPos, position.z, 1.0);
}