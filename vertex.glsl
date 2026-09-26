#version 430 core
#define SIDE_SIZE 128
#define SIDE_LENGTH (2.0 / SIDE_SIZE)

layout (location = 0) in vec3 position;
layout (binding = 0) buffer readBoard{
    uint board[];
};

out vec3 color;
int index = gl_InstanceID;
void main()
{
    int x = index % 128;
    int y = index / 128;
    color = board[index] ? vec3(1.0, 1.0, 1.0) : vec3(0.0, 0.0, 0.0);

    vec2 finalPos = position.xy + vec2((x + 0.5) * SIDE_LENGTH - 1, (y + 0.5) * SIDE_LENGTH - 1);
    gl_Position = vec4(finalPos, position.z, 1.0);
}