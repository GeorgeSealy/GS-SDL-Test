//
//  main.cpp
//  GL_SDL
//
//  Created by George Sealy on 22/07/16.
//  Copyright © 2016 MixBit. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
/* If using gl3.h */
/* Ensure we are using opengl's core profile only */
#define GL3_PROTOTYPES 1

#include <SDL2/SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <mach/mach_time.h>


#include "Maths.cpp"
#include "Utils.cpp"
#include "GLUtils.cpp"

#define PROGRAM_NAME "GL Skeleton"

/* A simple function that prints a message, the error code returned by SDL,
 * and quits the application */
void sdldie(const char *msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}


void checkSDLError(int line = -1)
{
#ifndef NDEBUG
    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        printf("SDL Error: %s\n", error);
        if (line != -1)
            printf(" + line: %i\n", line);
        SDL_ClearError();
    }
#endif
}

struct Mesh {
    GLuint vao = 0;
    GLuint vertexBuffer[2] = {0, 0};
    GLuint indexBuffer = 0;
    int numVertices = 0;
    int numIndices = 0;
    
    void setup(int numVerts, float *verts, float *colors, int numInds, ushort *indices) {
        
        if (vao == 0) {
            printf("Mesh setup\n");
            glGenVertexArrays(1, &vao);
            printf("VAO: %u\n", vao);
            
            glGenBuffers(2, vertexBuffer);
            printf("VBOs: %u, %u\n", vertexBuffer[0], vertexBuffer[1]);
            
            glGenBuffers(1, &indexBuffer);
            printf("IBO: %u\n", indexBuffer);
        }
        
        glBindVertexArray(vao);
        
        numVertices = numVerts;
        numIndices = numInds;
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 4 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);
    }
    
    void draw() {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
        glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_SHORT, 0);
    }
};

GLuint vertexShader = 0;
GLuint geometryShader = 0;
GLuint fragmentShader = 0;
GLuint shaderProgram = 0;
GLuint vao, vbo[3];

const GLfloat verts[4][4] = {
//    {  1.0,  1.0,  1.0,  1.0  },
//    { 0.5, 0.5,  1.0,  1.0  },
//    { 0.5,  1.0, -1.0,  1.0  },
//    {  1.0, 0.5, -1.0,  1.0  },
    {  0.5,  0.5,  0.0,  1.0  },
    { -0.5, -0.5,  0.0,  1.0  },
    { -0.5,  0.5,  0.0,  1.0  },
    {  0.5, -0.5,  0.0,  1.0  },
//    {  0.5,  0.5,  -5.0,  1.0  },
//    { -0.5, -0.5,  -5.0,  1.0  },
//    { -0.5,  0.5,  -5.0,  1.0  },
//    {  0.5, -0.5,  -5.0,  1.0  },
//    {  1.0,  1.0,  1.0,  1.0  },
//    { -1.0, -1.0,  1.0,  1.0  },
//    { -1.0,  1.0, -1.0,  1.0  },
//    {  1.0, -1.0, -1.0,  1.0  },
};

const GLfloat colors[4][3] = {
    {  1.0,  0.0,  0.0  },
    {  0.0,  1.0,  0.0  },
    {  0.0,  0.0,  1.0  },
    {  1.0,  1.0,  1.0  },
};

const GLushort indices[6] = { 0, 1, 2, 3, 0, 1 };

Mesh mesh;

void setupGL() {
    mesh.setup(4, (float *)verts, (float *)colors, 6, (ushort *)indices);

    vertexShader = compileShader("Assets/Shaders/SimpleCameraVertex.glsl", ShaderTypeVertex);
//    geometryShader = compileShader("Assets/Shaders/SimpleCameraGeometry.glsl", ShaderTypeGeometry);
    fragmentShader = compileShader("Assets/Shaders/SimpleCameraFragment.glsl", ShaderTypeFragment);
    
    printf("Shaders: %u, %u, %u\n", vertexShader, geometryShader, fragmentShader);
    
    shaderProgram = linkShaders(vertexShader, geometryShader, fragmentShader);
    printf("Program: %u\n", shaderProgram);
    
    glUseProgram(shaderProgram);
    
}

Timer timer = Timer();

void initWindowAndContext(SDL_Window **window, SDL_GLContext *context) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        sdldie("Unable to initialize SDL"); /* Or die on error */
    
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    /* Create our window centered at 512x512 resolution */
    *window = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!*window) /* Die if creation failed */
        sdldie("Unable to create window");
    
    checkSDLError(__LINE__);
    
    /* Create our opengl context and attach it to our window */
    *context = SDL_GL_CreateContext(*window);
    checkSDLError(__LINE__);
    
    SDL_GL_SetSwapInterval(1);
}

void deleteWindowAndContext(SDL_Window *window, SDL_GLContext &context) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}


void runMainLoop(SDL_Window *window) {
    
    bool quit = false;
    
    setupGL();
    
    double t = 0.0;
    const double dt = 1.0 / 60.0;
    
    double currentTime = timer.seconds();
    double accumulator = 0.0;
    
    double r = 0.0;
    double angle = 0.0;

    // Basic run loop from http://gafferongames.com/game-physics/fix-your-timestep/
    // TODO: (George) Final interpolation between states for super smoothness
    
    while (!quit)
    {
        double newTime = timer.seconds();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - START
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - END
        
        accumulator += frameTime;
        
        while ( accumulator >= dt )
        {
            
            // GAME STATE UPDATE - START
            //            integrate( state, t, dt );
            r += dt;
            
            if (r > 0.4) {
                r = 0.0;
            }
            
            
            angle += dt * 30.0;
            
            // GAME STATE UPDATE - END
            
            accumulator -= dt;
            t += dt;
        }
        
        // GAME STATE RENDER - START

        
//        render( state );
        glClearColor ( 0.2, 0.2, 0.4, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        double radAngle = degToRad(angle);
        double cs = cos(radAngle);
        double sn = sin(radAngle);
        
        buildProjectionMatrix(45.0, 4.0 / 3.0, 0.1, 100.0);
        setCamera(10.0 * cs, 10.0 * sn, 10.0, 0.0, 0.0, 0.0);
//        multMatrix(viewMatrix, projMatrix);
        multMatrix(projMatrix, viewMatrix);
        
//        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, projMatrix);
        
        glUseProgram(shaderProgram);
        
        mesh.draw();
        
        SDL_GL_SwapWindow(window);
        
        // GAME STATE RENDER - END
    }

    // Clean up GL
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    
}

/* Our program's entry point */
int main(int argc, char *argv[])
{
    SDL_Window *mainWindow; /* Our window handle */
    SDL_GLContext mainContext; /* Our opengl context handle */

    initWindowAndContext(&mainWindow, &mainContext);
    runMainLoop(mainWindow);
    deleteWindowAndContext(mainWindow, mainContext);

    SDL_Quit();
    
    return 0;
}