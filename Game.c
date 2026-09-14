#include <string.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdlib.h>

#define SIDE_SIZE 128
#define SIZE SIDE_SIZE * SIDE_SIZE

#define Up(y)    (((y) + 1) % SIDE_SIZE)
#define Down(y)  (((y) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Right(x) (((x) + 1) % SIDE_SIZE)
#define Left(x)  (((x) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Still(x_OR_y) (x_OR_y)

#define SIDE_LENGTH (2.0f / SIDE_SIZE)
#define HALF_SIDE (1.0f / SIDE_SIZE)

int board[2][SIDE_SIZE][SIDE_SIZE] = {0};

int remembered[SIDE_SIZE][SIDE_SIZE] = {0};

int listNumber = 0;
int cellsToCheck[2][SIZE][2] = {0};
int checkLength[2] = {0};

int checkNeighbors(int x, int y, int listNumber){
    int liveNeighbors = 0;
    liveNeighbors += board[listNumber][Still(x)][Up(y)];
    liveNeighbors += board[listNumber][Left(x)][Up(y)];
    liveNeighbors += board[listNumber][Right(x)][Up(y)];
    
    liveNeighbors += board[listNumber][Still(x)][Down(y)];
    liveNeighbors += board[listNumber][Left(x)][Down(y)];
    liveNeighbors += board[listNumber][Right(x)][Down(y)];
    
    liveNeighbors += board[listNumber][Left(x)][Still(y)];
    liveNeighbors += board[listNumber][Right(x)][Still(y)];

    return liveNeighbors;
}

void addCellToCheck(int x, int y, int listNumber){
    cellsToCheck[listNumber][checkLength[listNumber]][0] = x;
    cellsToCheck[listNumber][checkLength[listNumber]++][1] = y;
}

void rememberAll(){
    for(int y = 0; y < SIDE_SIZE; y++){
        for(int x = 0; x < SIDE_SIZE; x++){
            addCellToCheck(x, y, listNumber);
        }
    }
}

void remember(int x, int y, int listNumber){
    addCellToCheck(x, y, listNumber);
    
    addCellToCheck(Still(x), Up(y), listNumber);
    addCellToCheck(Left(x), Up(y), listNumber);
    addCellToCheck(Right(x), Up(y), listNumber);
    
    addCellToCheck(Still(x), Down(y), listNumber);
    addCellToCheck(Left(x), Down(y), listNumber);
    addCellToCheck(Right(x), Down(y), listNumber);
    
    addCellToCheck(Right(x), Still(y), listNumber);
    addCellToCheck(Left(x), Still(y), listNumber);
}

void gameLoop(){
    for(int i = 0; i < checkLength[listNumber]; i++){
        int x = cellsToCheck[listNumber][i][0];
        int y = cellsToCheck[listNumber][i][1];
        
        if(remembered[x][y] == 1)
            continue;
        remembered[x][y] = 1;

        int neighbors = checkNeighbors(x, y, listNumber);

        if(board[listNumber][x][y]){
            if(neighbors < 2 || neighbors > 3){
                board[listNumber ^ 1][x][y] = 0;
            }
            else{
                board[listNumber ^ 1][x][y] = 1;
            }
        }
        else if(neighbors == 3){
            board[listNumber ^ 1][x][y] = 1;
        }
        
        if(board[listNumber ^ 1][x][y] == 1){
            remember(x, y, listNumber ^ 1);
        }
    }
    memset(remembered, 0, sizeof(remembered));
    memset(board[listNumber], 0, sizeof(board[listNumber]));
    
    checkLength[listNumber] = 0;
    listNumber = listNumber ^ 1;
}

int generateGridPositions(float sideLength, float* buffer){ // 2 must be divisble by sideLength
    int numSquaresX = 2 / sideLength;
    int numSquaresY = numSquaresX;
    
    int alive = 0;

    for(int y = 0; y < numSquaresY; y++){
        for(int x = 0; x <numSquaresX; x++){
            if(!board[listNumber][x][y])
                continue;

            int index = alive * 2;
            buffer[index] = -1.0 + (x + 0.5) * sideLength;
            buffer[index + 1] = -1.0 + (y + 0.5) * sideLength;
            alive++;
        }
    }

    return alive;
}

float vertices[] = {
    -HALF_SIDE,  HALF_SIDE, 0.00, 1.0, 1.0, 1.0, // top left
     HALF_SIDE,  HALF_SIDE, 0.00, 1.0, 1.0, 1.0, //top right
     HALF_SIDE, -HALF_SIDE, 0.00, 1.0, 1.0, 1.0, // bottom right
    -HALF_SIDE, -HALF_SIDE, 0.00, 1.0, 1.0, 1.0, //bottome left
};

unsigned int indices[] = {
    0, 1, 2,
    0, 3, 2,
};

float* positions;

GLuint vertexShader;

const char *vertexSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 colorIn;\n"
    "layout (location = 2) in vec2 instancePos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
        "color = colorIn;\n"
        "vec2 finalPos = position.xy + instancePos;\n"
        "gl_Position = vec4(finalPos, position.z, 1.0);\n"
    "}\n";

const char *fragmentSource =
        "#version 330 core\n"
        "in vec3 color;\n"
        "out vec4 colorOut;\n"
        "void main()\n"
        "{\n"
            "colorOut = vec4(color, 1.0);\n"
        "}\n";

GLuint createShaderProgram(){
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void createVBO(GLuint* VBO, size_t dataSize, void* data, GLenum usage){
    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
}

void configureAttribPointer(GLuint VBO, GLuint index, GLint size, GLenum type, GLsizei stride, void* offset){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, offset); // last arg is offset
    glEnableVertexAttribArray(index);
}

int main(){
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 800, "Conway's Game of Life", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    
    board[listNumber][51][50] = 1;
    board[listNumber][50][50] = 1;
    board[listNumber][52][50] = 1;
    rememberAll();
    positions = malloc(SIZE * 2 * sizeof(float));
    int alive = generateGridPositions(SIDE_LENGTH, positions);

    GLuint shaderProgram = createShaderProgram();

    GLuint VerticesBuffer;
    GLuint PositionsBuffer;
    GLuint EBO;

    GLuint VAO;
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    
    glBindVertexArray(VAO);
    createVBO(&VerticesBuffer, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    configureAttribPointer(VerticesBuffer, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    configureAttribPointer(VerticesBuffer, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(sizeof(float) * 3));
    
    createVBO(&PositionsBuffer, sizeof(float) * SIZE * 2, positions, GL_DYNAMIC_DRAW);
    configureAttribPointer(PositionsBuffer, 2, 2, GL_FLOAT, sizeof(float) * 2, (void*)0);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, PositionsBuffer);
    
    double lastTime = 0;
    double delay = 0.1;
    while(!glfwWindowShouldClose(window)){
        if(glfwGetTime() - lastTime > delay){
            lastTime = glfwGetTime();
            
            gameLoop();
            alive = generateGridPositions(SIDE_LENGTH, positions);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * alive * 2, positions);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, alive);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(positions);
    glfwTerminate();
    return 0;
}
