#version 330 core

in vec3 color;
in vec3 localPos;
out vec4 colorOut;

void main()
{
    colorOut = vec4(color, 0.0);

    if(abs(localPos.x) > 1.0/128 - 0.001 || abs(localPos.y) > 1.0/128 - 0.001){
        colorOut = vec4(0.0, 0.0, 0.0, 1.0);
    }
}