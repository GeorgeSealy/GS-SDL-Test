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
#include "Mesh.cpp"

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

GLuint vertexShader = 0;
GLuint geometryShader = 0;
GLuint fragmentShader = 0;
GLuint shaderProgram = 0;

Mesh buildGridMesh(int squaresPerSide, int axis, bool flipped) {
    
    float flipMultiplier = flipped ? -1.0 : 1.0;
    
    float sx = flipMultiplier * -1.0;
    float sy = flipMultiplier * -1.0;
    float sz = flipMultiplier;
    
    float incx = flipMultiplier * 2.0 / squaresPerSide;
    float incy = flipMultiplier * 2.0 / squaresPerSide;
    
    int axisX = (axis + 1) % 3;
    int axisY = (axis + 2) % 3;
    int axisZ = axis % 3;
    
    int numVerts = (squaresPerSide + 1) * (squaresPerSide + 1);

    float *verts = (float *)malloc(sizeof(float) * 3 * numVerts);
    float *cols = (float *)malloc(sizeof(float) * 3 * numVerts);

    int vertIndex = 0;
    for (int j = 0; j <= squaresPerSide; ++j) {
        float y = sy + j * incy;
        
        for (int i = 0; i <= squaresPerSide; ++i) {
            
            float x = sx + i * incx;
            
            verts[vertIndex + axisX] = x;
            verts[vertIndex + axisY] = y;
            verts[vertIndex + axisZ] = sz;
            
            normalize(&verts[vertIndex]);
            
            float scaleFactor = 1.0 + drand48() * 0.2;
            
            verts[vertIndex + 0] *= scaleFactor;
            verts[vertIndex + 1] *= scaleFactor;
            verts[vertIndex + 2] *= scaleFactor;
            
            cols[vertIndex + axisX] = ((j % 2) == 0) ? 0.0 : 1.0;
            cols[vertIndex + axisY] = ((i % 2) == 0) ? 0.0 : 1.0;
            cols[vertIndex + axisZ] = 0.0;
            
            vertIndex += 3;
        }
    }
    
    int numIndices = (2 * (squaresPerSide + 1)) * squaresPerSide;
    ushort *indices = (ushort *)malloc(sizeof(ushort) * numIndices);

    int index = 0;
    
    for (int j = 0; j < squaresPerSide; ++j) {
        for (int i = 0; i <= squaresPerSide; ++i) {
            
            if ((j % 2) == 0) {
                
                ushort baseIndex = j * (squaresPerSide + 1) + i;
                
                indices[index + 0] = baseIndex;
                indices[index + 1] = baseIndex + (squaresPerSide + 1);
                
            } else {
                
                ushort baseIndex = j * (squaresPerSide + 1) + (squaresPerSide - i);
                
                indices[index + 0] = baseIndex + (squaresPerSide + 1);
                indices[index + 1] = baseIndex;
                
            }
            
            index += 2;
        }
    }
    
    Mesh result;
    
    result.setup(numVerts, verts, cols, numIndices, indices);
    
    return result;
}

Mesh posXMesh;
Mesh negXMesh;
Mesh posYMesh;
Mesh negYMesh;
Mesh posZMesh;
Mesh negZMesh;

void setupGL() {
    
    posXMesh = buildGridMesh(32, 0, false);
    negXMesh = buildGridMesh(32, 0, true);
    posYMesh = buildGridMesh(32, 1, false);
    negYMesh = buildGridMesh(32, 1, true);
    posZMesh = buildGridMesh(32, 2, false);
    negZMesh = buildGridMesh(32, 2, true);

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
        setCamera(4.0 * cs, 4.0 * sn, 1.5, 0.0, 0.0, 0.0);
        multMatrix(projMatrix, viewMatrix);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, projMatrix);
        
        glUseProgram(shaderProgram);
        
        posXMesh.draw();
        negXMesh.draw();
        posYMesh.draw();
        negYMesh.draw();
        posZMesh.draw();
        negZMesh.draw();
        
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