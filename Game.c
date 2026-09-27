#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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

int curBoard = 0;
int cellsToCheck[2][SIZE][2] = {0};
int checkLength[2] = {0};

int runMode = 0;
int cursor[2] = {64, 64};

void ReadFile(const char *path, char* buffer){
    FILE *file = fopen(path, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
}

int checkNeighbors(int x, int y, int curBoard){
    int liveNeighbors = 0;
    liveNeighbors += board[curBoard][Still(x)][Up(y)];
    liveNeighbors += board[curBoard][Left(x)][Up(y)];
    liveNeighbors += board[curBoard][Right(x)][Up(y)];
    
    liveNeighbors += board[curBoard][Still(x)][Down(y)];
    liveNeighbors += board[curBoard][Left(x)][Down(y)];
    liveNeighbors += board[curBoard][Right(x)][Down(y)];
    
    liveNeighbors += board[curBoard][Left(x)][Still(y)];
    liveNeighbors += board[curBoard][Right(x)][Still(y)];

    return liveNeighbors;
}

void addCellToCheck(int x, int y, int curBoard){
    cellsToCheck[curBoard][checkLength[curBoard]][0] = x;
    cellsToCheck[curBoard][checkLength[curBoard]++][1] = y;
}

void rememberAll(){
    for(int y = 0; y < SIDE_SIZE; y++){
        for(int x = 0; x < SIDE_SIZE; x++){
            addCellToCheck(x, y, curBoard);
        }
    }
}

void remember(int x, int y, int curBoard){
    addCellToCheck(x, y, curBoard);
    
    addCellToCheck(Still(x), Up(y), curBoard);
    addCellToCheck(Left(x), Up(y), curBoard);
    addCellToCheck(Right(x), Up(y), curBoard);
    
    addCellToCheck(Still(x), Down(y), curBoard);
    addCellToCheck(Left(x), Down(y), curBoard);
    addCellToCheck(Right(x), Down(y), curBoard);
    
    addCellToCheck(Right(x), Still(y), curBoard);
    addCellToCheck(Left(x), Still(y), curBoard);
}

void gameLoop(){
    for(int i = 0; i < checkLength[curBoard]; i++){
        int x = cellsToCheck[curBoard][i][0];
        int y = cellsToCheck[curBoard][i][1];
        
        if(remembered[x][y] == 1)
            continue;
        remembered[x][y] = 1;

        int neighbors = checkNeighbors(x, y, curBoard);

        if(board[curBoard][x][y]){
            if(neighbors < 2 || neighbors > 3){
                board[curBoard ^ 1][x][y] = 0;
            }
            else{
                board[curBoard ^ 1][x][y] = 1;
            }
        }
        else if(neighbors == 3){
            board[curBoard ^ 1][x][y] = 1;
        }
        
        if(board[curBoard ^ 1][x][y] == 1){
            remember(x, y, curBoard ^ 1);
        }
    }
    memset(remembered, 0, sizeof(remembered));
    memset(board[curBoard], 0, sizeof(board[curBoard]));
    
    checkLength[curBoard] = 0;
    curBoard = curBoard ^ 1;
}

int generateGridPositions(float sideLength, float* allSquares, float* aliveSquares){ // 2 must be divisble by sideLength
    int alive = 0;
    int index = alive * 2;

    for(int y = 0; y < SIDE_SIZE; y++){
        for(int x = 0; x < SIDE_SIZE; x++){
            if(!board[curBoard][x][y]){
                index = (x + y * SIDE_SIZE) * 2;
                allSquares[index] = -1.0 + (x + 0.5) * sideLength;
                allSquares[index + 1] = -1.0 + (y + 0.5) * sideLength;
                continue;
            }

            index = alive * 2;
            aliveSquares[index] = -1.0 + (x + 0.5) * sideLength;
            aliveSquares[index + 1] = -1.0 + (y + 0.5) * sideLength;
            alive++;

            index = (x + y * SIDE_SIZE) * 2;
            allSquares[index] = -1.0 + (x + 0.5) * sideLength;
            allSquares[index + 1] = -1.0 + (y + 0.5) * sideLength;
        }
    }

    return alive;
}

float vertices[] = {
    -HALF_SIDE,  HALF_SIDE, 0.00, // top left
     HALF_SIDE,  HALF_SIDE, 0.00, //top right
     HALF_SIDE, -HALF_SIDE, 0.00, // bottom right
    -HALF_SIDE, -HALF_SIDE, 0.00, //bottome left
};

unsigned int indices[] = {
    0, 1, 2,
    0, 3, 2,
};

char *outlineVertex;
char *outlineFragment;
char *cursorFragment;
char *vertexSource;
char *fragmentSource;

char *computeSource;

GLuint createShaderProgram(const char *const vertexSource, const char *const fragmentSource){
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
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

void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods){
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && action == GLFW_PRESS){
        runMode ^= 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && action == GLFW_PRESS && !runMode){
        for(int y = 0; y < SIDE_SIZE; y++){
            for(int x = 0; x < SIDE_SIZE; x++){
                board[curBoard][x][y] = 0;                
            }
        }
    }
    else if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && action == GLFW_PRESS && !runMode){
        board[curBoard][cursor[0]][cursor[1]] ^= 1;
        remember(cursor[0], cursor[1], curBoard);
    }
    else if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && action == GLFW_PRESS){
        cursor[1] += 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && action == GLFW_PRESS){
        cursor[1] -= 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && action == GLFW_PRESS){
        cursor[0] -= 1;
    }
    else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && action == GLFW_PRESS){
        cursor[0] += 1;
    }
}

int main(){
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(768, 768, "Conway's Game of Life", NULL, NULL);
    glfwSetKeyCallback(window, keyCallBack);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ReadFile("vertexOutline.glsl", outlineVertex);
    ReadFile("fragmentOutline.glsl", outlineFragment);
    ReadFile("fragmentCursor.glsl", cursorFragment);
    ReadFile("vertex.glsl", vertexSource);
    ReadFile("fragment.glsl", fragmentSource);
    GLuint cellProgram = createShaderProgram(vertexSource, fragmentSource);
    GLuint outlineProgram = createShaderProgram(outlineVertex, outlineFragment);
    GLuint cursorProgram = createShaderProgram(outlineVertex, cursorFragment);

    ReadFile("compute.glsl", computeSource);
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, (const char *const *)computeSource, NULL);
    glCompileShader(computeShader);

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glDeleteShader(computeShader);

    GLuint boardBuffers[2];
    glGenBuffers(2, boardBuffers);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board), board, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board), board, GL_DYNAMIC_DRAW);

    GLuint VerticesBuffer;
    GLuint EBO;
    
    GLuint VAO;
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
    
    glBindVertexArray(VAO);
    createVBO(&VerticesBuffer, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    configureAttribPointer(VerticesBuffer, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    double lastTime = 0;
    double delay = 0.1;
    while(!glfwWindowShouldClose(window)){

        if(runMode){
            if(glfwGetTime() - lastTime > delay){
                lastTime = glfwGetTime();
                
                glUseProgram(computeProgram);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, boardBuffers[curBoard]);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, boardBuffers[curBoard ^ 1]);
                glDispatchCompute(16, 16, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                curBoard ^= 1;
            }
        }
        else{
        }
        
        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBindVertexArray(VAO);
        
        glUseProgram(cellProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, SIZE);
        
        glUseProgram(outlineProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, SIZE);
        
        float cursorPosition[2] = {(cursor[0] + 0.5) * SIDE_LENGTH - 1, (cursor[1] + 0.5) * SIDE_LENGTH - 1};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 2, cursorPosition);
        glUseProgram(cursorProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
