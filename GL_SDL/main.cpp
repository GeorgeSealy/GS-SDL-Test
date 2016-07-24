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
const float planetRadius = 6000.0;

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

    Vec3 *verts = (Vec3 *)malloc(sizeof(Vec3) * numVerts);
    Vec3 *cols = (Vec3 *)malloc(sizeof(Vec3) * numVerts);

    int vertIndex = 0;
    for (int j = 0; j <= squaresPerSide; ++j) {
        float y = sy + j * incy;
        
        for (int i = 0; i <= squaresPerSide; ++i) {
            
            float x = sx + i * incx;
            
            verts[vertIndex][axisX] = x;
            verts[vertIndex][axisY] = y;
            verts[vertIndex][axisZ] = sz;
            
            normalize(verts[vertIndex]);
            
            float scaleFactor = verts[vertIndex][0] * verts[vertIndex][1] * verts[vertIndex][2];
            
            scaleFactor *= 10000.0;
            scaleFactor = 5.0f * fmodf(fabsf(scaleFactor), 0.2f);
            
            scaleFactor = planetRadius * (1.0 + scaleFactor * 0.025);
            
            verts[vertIndex][0] *= scaleFactor;
            verts[vertIndex][1] *= scaleFactor;
            verts[vertIndex][2] *= scaleFactor;
            
            cols[vertIndex][axisX] = ((j % 2) == 0) ? 0.0 : 1.0;
            cols[vertIndex][axisY] = ((i % 2) == 0) ? 0.0 : 1.0;
            cols[vertIndex][axisZ] = 0.0;
            
            vertIndex += 1;
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
    
    free(verts);
    free(cols);
    free(indices);
    
    return result;
}

Mesh posXMesh;
Mesh negXMesh;
Mesh posYMesh;
Mesh negYMesh;
Mesh posZMesh;
Mesh negZMesh;

void setupGL() {
    
    posXMesh = buildGridMesh(128, 0, false);
    negXMesh = buildGridMesh(128, 0, true);
    posYMesh = buildGridMesh(128, 1, false);
    negYMesh = buildGridMesh(128, 1, true);
    posZMesh = buildGridMesh(128, 2, false);
    negZMesh = buildGridMesh(128, 2, true);

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

struct PointOfView {

    Vec3 position;
    Vec3 direction, right, up;
    
    void setPosition(Vec3 pos) {
        memcpy(position, pos, sizeof(Vec3));
    }
    
    void setDirection(Vec3 dir) {
        memcpy(direction, dir, sizeof(Vec3));
    }
    
    void updateForUpVector(Vec3 referenceUp) {
        normalize(direction);
        
        crossProduct(direction, referenceUp, right);
        normalize(right);
        
        crossProduct(right, direction, up);
        normalize(up);

    }
    
    void getCameraMatrix(Mat4x4 &viewMatrix) {
        
        viewMatrix[0]  = right[0];
        viewMatrix[4]  = right[1];
        viewMatrix[8]  = right[2];
        viewMatrix[12] = 0.0f;
        
        viewMatrix[1]  = up[0];
        viewMatrix[5]  = up[1];
        viewMatrix[9]  = up[2];
        viewMatrix[13] = 0.0f;
        
        viewMatrix[2]  = -direction[0];
        viewMatrix[6]  = -direction[1];
        viewMatrix[10] = -direction[2];
        viewMatrix[14] =  0.0f;
        
        viewMatrix[3]  = 0.0f;
        viewMatrix[7]  = 0.0f;
        viewMatrix[11] = 0.0f;
        viewMatrix[15] = 1.0f;
        
        Mat4x4 aux;
        setTranslationMatrix(aux, -position[0], -position[1], -position[2]);
        
        multMatrix(viewMatrix, aux);
    }
};

void runMainLoop(SDL_Window *window) {
    
    bool quit = false;
    
    setupGL();
    
    double t = 0.0;
    const double dt = 1.0 / 60.0;
    
    double currentTime = timer.seconds();
    double accumulator = 0.0;
    
    double r = 0.0;
    double angle = 0.0;

    PointOfView view;
    
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
            
            
            angle += dt * 1.0;
            
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
        float cs = cos(radAngle);
        float sn = sin(radAngle);
        
        Mat4x4 projectionMatrix;
        Mat4x4 viewMatrix;
        
        buildProjectionMatrix(projectionMatrix, 45.0, 4.0 / 3.0, 0.1, 10000.0);
        
        Vec3 from = {1.05f * cs, 1.05f * sn, 0.2};
        Vec3 dir = {-from[1] - 0.4f * from[0], from[0] - 0.4f * from[1], 0.4f *  from[2]};
        
        from[0] *= planetRadius;
        from[1] *= planetRadius;
        from[2] *= planetRadius;
        
        dir[0] *= planetRadius;
        dir[1] *= planetRadius;
        dir[2] *= planetRadius;
        
        view.setPosition(from);
        view.setDirection(dir);
        view.updateForUpVector(from);
        
        view.getCameraMatrix(viewMatrix);
        
        multMatrix(projectionMatrix, viewMatrix);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, projectionMatrix);
        
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