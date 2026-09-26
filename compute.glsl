#version 330 core

#define SIDE_SIZE 128
#define SIZE SIDE_SIZE * SIDE_SIZE

#define Up(y)    (((y) + 1) % SIDE_SIZE)
#define Down(y)  (((y) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Right(x) (((x) + 1) % SIDE_SIZE)
#define Left(x)  (((x) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Still(x_OR_y) (x_OR_y)

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0) buffer readBoard{
    uint readValues[];
};
layout (binding = 1) buffer writeBoard{
    uint writeValues[];
};

uint x = gl_GlobalInvocationID.x;
uint y = gl_GlobalInvocationID.y;

void main(){
    int index = x + y * SIDE_SIZE;
    int liveNeighbors = 0;

    index = Still(x) + Up(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    index = Left(x) + Up(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    index = Right(x) + Up(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    
    index = Still(x) + Down(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    index = Left(x) + Down(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    index = Right(x) + Down(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    
    index = Left(x) + Still(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];
    index = Right(x) + Still(y) * SIDE_SIZE;
    liveNeighbors += readValues[index];

    index = x + y * SIDE_SIZE;
    writeValues[index] = readValues[index];
    if(liveNeighbors < 2 || liveNeighbors > 3){
        writeValues[index] = 0;
    }
    else if(liveNeighbors == 3){
        writeValues[index] = 1;
    }
}