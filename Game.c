#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define SIDE_SIZE 128
#define SIZE SIDE_SIZE * SIDE_SIZE
#define SHADER_BUFFER_SIZE 1700

#define Up(y)    (((y) + 1) % SIDE_SIZE)
#define Down(y)  (((y) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Right(x) (((x) + 1) % SIDE_SIZE)
#define Left(x)  (((x) - 1 + SIDE_SIZE) % SIDE_SIZE)
#define Still(x_OR_y) (x_OR_y)

#define SIDE_LENGTH (2.0f / SIDE_SIZE)
#define HALF_SIDE (1.0f / SIDE_SIZE)

int board[2][SIDE_SIZE][SIDE_SIZE] = {0};

int curBoard = 0;
GLuint boardBuffers[2];

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

char outlineVertex[SHADER_BUFFER_SIZE];
char outlineFragment[SHADER_BUFFER_SIZE];
char cursorFragment[SHADER_BUFFER_SIZE];
char cursorVertex[SHADER_BUFFER_SIZE];
char vertexSource[SHADER_BUFFER_SIZE];
char fragmentSource[SHADER_BUFFER_SIZE];

char computeSource[SHADER_BUFFER_SIZE];

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

GLuint createComputeShader(const char *computeSource){
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeSource, NULL);
    glCompileShader(computeShader);

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glDeleteShader(computeShader);

    return computeProgram;
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
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[curBoard]);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, SIZE * sizeof(int), board[curBoard]);
    }
    else if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && action == GLFW_PRESS && !runMode){
        board[curBoard][cursor[0]][cursor[1]] ^= 1;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[curBoard]);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, (cursor[0] + cursor[1] * SIDE_SIZE) * sizeof(int), sizeof(int), &board[curBoard][cursor[0]][cursor[1]]);
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
    ReadFile("vertexCursor.glsl", cursorVertex);
    ReadFile("vertex.glsl", vertexSource);
    ReadFile("fragment.glsl", fragmentSource);
    GLuint cellProgram = createShaderProgram(vertexSource, fragmentSource);
    GLuint outlineProgram = createShaderProgram(outlineVertex, outlineFragment);
    GLuint cursorProgram = createShaderProgram(cursorVertex, cursorFragment);

    ReadFile("compute.glsl", computeSource);
    GLuint computeProgram = createComputeShader(computeSource);

    glGenBuffers(2, boardBuffers);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[curBoard]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board[curBoard]), board[curBoard], GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, boardBuffers[curBoard ^ 1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(board[curBoard]), board[curBoard], GL_DYNAMIC_DRAW);

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

    GLuint cursorPositionBuffer;
    GLuint cursorVertexAttribute;
    GLuint curosrElementBuffer;
   
    glGenBuffers(1, &curosrElementBuffer);
    glGenVertexArrays(1, &cursorVertexAttribute);

    glBindVertexArray(cursorVertexAttribute);
    configureAttribPointer(VerticesBuffer, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    
    float cursorPos[2] = {(cursor[0] + 0.5) * SIDE_LENGTH - 1, (cursor[1] + 0.5) * SIDE_LENGTH - 1};
    createVBO(&cursorPositionBuffer, sizeof(cursorPos), cursorPos, GL_DYNAMIC_DRAW);
    configureAttribPointer(cursorPositionBuffer, 1, 2, GL_FLOAT, 2 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curosrElementBuffer);
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
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, boardBuffers[curBoard]);
        
        glUseProgram(cellProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, SIZE);
        
        glUseProgram(outlineProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, SIZE);
        
        glBindVertexArray(cursorVertexAttribute);
        glBindBuffer(GL_ARRAY_BUFFER, cursorPositionBuffer);
        cursorPos[0] = (cursor[0] + 0.5) * SIDE_LENGTH - 1;
        cursorPos[1] = (cursor[1] + 0.5) * SIDE_LENGTH - 1;
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cursorPos), cursorPos);
        glUseProgram(cursorProgram);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
