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
#include <math.h>

#include "Maths.cpp"
#include "Utils.cpp"
#include "GLUtils.cpp"
#include "Mesh.cpp"
#include "Noise.cpp"

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

inline float normalizedHeightAboveSeaLevel(const v3 &basePt) {
    v3 pt = v3normalize(basePt);
    
    double n = noise(4.0 * pt.x, 4.0 * pt.y, 4.0 * pt.z);
    double n2 = noise(7.0 * pt.x, 7.0 * pt.y, 7.0 * pt.z);
    
    float height = n + 0.5 * n2;
    
    return height;
}

Mesh buildGridMesh(int squaresPerSide, v3 startPt, v3 acrossDir, v3 upDir) {
    
    float scaleFactor = 1.0 / squaresPerSide;
    
    v3 uStep = scaleFactor * acrossDir;
    v3 vStep = scaleFactor * upDir;
    
    int numVerts = (squaresPerSide + 1) * (squaresPerSide + 1);

    v3 *verts = (v3 *)malloc(sizeof(v3) * numVerts);
    v3 *norms = (v3 *)malloc(sizeof(v3) * numVerts);
    v3 *cols = (v3 *)malloc(sizeof(v3) * numVerts);

    int vertIndex = 0;
    for (int j = 0; j <= squaresPerSide; ++j) {
        
        for (int i = 0; i <= squaresPerSide; ++i) {
            
            v3 &pt = verts[vertIndex];
            
            v3 u = i * uStep;
            v3 v = j * vStep;

            pt = startPt + u + v;

//            // TODO: (George) Move the normalisation / height code out of here into a separate function
//            // TODO: (George) Will require an "update mesh" function and stored arrays in mesh struct
            pt = v3normalize(pt);
            
            float height = normalizedHeightAboveSeaLevel(pt);
            
            height += 0.1;
            
            double multiplier = 0.125;
            float distFromCentre = planetRadius * (1.0 + height * multiplier);
            
            if (height < 0.0) {
                distFromCentre = planetRadius;
            }
            
            pt = distFromCentre * pt;
            
            if (height < 0.0) {
                cols[vertIndex].r = 0.2;
                cols[vertIndex].g = 0.2 - height * 0.5;
                cols[vertIndex].b = 1.0;
            } else if (height < 0.8) {
                cols[vertIndex].r = 0.2 + 0.4 * height;
                cols[vertIndex].g = 0.5;
                cols[vertIndex].b = 0.1 + 0.4 * height;
            } else {
                cols[vertIndex].r = 0.9;
                cols[vertIndex].g = 0.9;
                cols[vertIndex].b = 0.9;
            }
            
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
    
    result.setup(numVerts, verts, norms, cols, numIndices, indices);
    
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
    
    int numSquaresPerSide = 128;
    
    v3 startPt(1.0, -1.0, -1.0);
    v3 across(0.0, 2.0, 0.0);
    v3 up(0.0, 0.0, 2.0);
    
    posXMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);
    
    startPt = v3(-1.0, 1.0, -1.0);
    across = v3(0.0, -2.0, 0.0);
    
    negXMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);
    
    startPt = v3(1.0, 1.0, -1.0);
    across = v3(-2.0, 0.0, 0.0);
    
    posYMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);
    
    startPt = v3(-1.0, -1.0, -1.0);
    across = v3(2.0, 0.0, 0.0);
    
    negYMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);
    
    startPt = v3(-1.0, -1.0, 1.0);
    across = v3(2.0, 0.0, 0.0);
    up = v3(0.0, 2.0, 0.0);
    
    posZMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);
    
    startPt = v3(-1.0, -1.0, -1.0);
    across = v3(2.0, 0.0, 0.0);
//    up[0] = 0.0; up[1] = -2.0; up[2] = 0.0;
    
    negZMesh = buildGridMesh(numSquaresPerSide, startPt, across, up);


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

    v3 position;
    v3 direction, right, up;
    
    v3 rotatePointAboutLine(v3 p, double theta, v3 p1, v3 p2)
    {
        v3 u,q1,q2;
        double d;
        
        /* Step 1 */
        q1.x = p.x - p1.x;
        q1.y = p.y - p1.y;
        q1.z = p.z - p1.z;
        
        u.x = p2.x - p1.x;
        u.y = p2.y - p1.y;
        u.z = p2.z - p1.z;
        
        u = v3normalize(u);
//        Normalise(&u);
        d = sqrt(u.y*u.y + u.z*u.z);
        
        /* Step 2 */
        if (d != 0) {
            q2.x = q1.x;
            q2.y = q1.y * u.z / d - q1.z * u.y / d;
            q2.z = q1.y * u.y / d + q1.z * u.z / d;
        } else {
            q2 = q1;
        }
        
        /* Step 3 */
        q1.x = q2.x * d - q2.z * u.x;
        q1.y = q2.y;
        q1.z = q2.x * u.x + q2.z * d;
        
        /* Step 4 */
        q2.x = q1.x * cos(theta) - q1.y * sin(theta);
        q2.y = q1.x * sin(theta) + q1.y * cos(theta);
        q2.z = q1.z;
        
        /* Inverse of step 3 */
        q1.x =   q2.x * d + q2.z * u.x;
        q1.y =   q2.y;
        q1.z = - q2.x * u.x + q2.z * d;
        
        /* Inverse of step 2 */
        if (d != 0) {
            q2.x =   q1.x;
            q2.y =   q1.y * u.z / d + q1.z * u.y / d;
            q2.z = - q1.y * u.y / d + q1.z * u.z / d;
        } else {
            q2 = q1;
        }
        
        /* Inverse of step 1 */
        q1.x = q2.x + p1.x;
        q1.y = q2.y + p1.y;
        q1.z = q2.z + p1.z;
        return(q1);
    }
    
    void roll(double angle) {
        direction = v3normalize(direction);
        v3 newUp = rotatePointAboutLine(up, angle, v3(0.0, 0.0, 0.0), direction);
        
        updateForUpVector(newUp);
    }
    
    void pitch(double angle) {
        right = v3normalize(right);
        
        direction = rotatePointAboutLine(direction, angle, v3(0.0, 0.0, 0.0), right);
        v3 newUp = v3cross(right, direction);
        
        updateForUpVector(newUp);
    }
    
    void updateForUpVector(v3 referenceUp) {
        direction = v3normalize(direction);
        
        right = v3cross(direction, referenceUp);
        right = v3normalize(right);
        
        up = v3cross(right, direction);
        up = v3normalize(up);

    }
    
    void getCameraMatrix(Mat4x4 &viewMatrix) {
        
        viewMatrix[0]  = right.x;
        viewMatrix[4]  = right.y;
        viewMatrix[8]  = right.z;
        viewMatrix[12] = 0.0f;
        
        viewMatrix[1]  = up.x;
        viewMatrix[5]  = up.y;
        viewMatrix[9]  = up.z;
        viewMatrix[13] = 0.0f;
        
        viewMatrix[2]  = -direction.x;
        viewMatrix[6]  = -direction.y;
        viewMatrix[10] = -direction.z;
        viewMatrix[14] =  0.0f;
        
        viewMatrix[3]  = 0.0f;
        viewMatrix[7]  = 0.0f;
        viewMatrix[11] = 0.0f;
        viewMatrix[15] = 1.0f;
        
        Mat4x4 aux;
        setTranslationMatrix(aux, -position.x, -position.y, -position.z);
        
        multMatrix(viewMatrix, aux);
    }
};

struct InputState {
    InputState() {
        forwardActive = false;
        backActive = false;
        leftActive = false;
        rightActive = false;
        upActive = false;
        downActive = false;
        shouldQuit = false;
    }
    
    bool forwardActive;
    bool backActive;
    bool leftActive;
    bool rightActive;
    bool upActive;
    bool downActive;
    bool shouldQuit;
};

void runMainLoop(SDL_Window *window) {
    
    setupGL();
    
    double t = 0.0;
    const double dt = 1.0 / 60.0;
    
    double currentTime = timer.seconds();
    double accumulator = 0.0;
    
    double rollAcceleration = 0.5;
    double rollVelocity = 0.0;
    double rollMultiplier = 0.3;
    
    double pitchAcceleration = 0.5;
    double pitchVelocity = 0.0;
    double pitchMultiplier = 0.3;

    PointOfView view;
    
    v3 from(5.0, 0.0, 0.2);
    v3 dir(-from.x, -from.y, -from.z);
    v3 up(0.0, 0.0, 1.0);
    
    from = planetRadius * from;
    view.position = from;
    view.direction = dir;
    view.updateForUpVector(up);

    SDL_Event event;
    InputState inputs;
    
    // Basic run loop from http://gafferongames.com/game-physics/fix-your-timestep/
    // TODO: (George) Final interpolation between states for super smoothness
    
    while (!inputs.shouldQuit)
    {
        double newTime = timer.seconds();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - START
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym ) {
                        case SDLK_ESCAPE:
                            inputs.shouldQuit = true;
                            break;
                        case SDLK_w:
                            inputs.upActive = true;
                            break;
                        case SDLK_s:
                            inputs.downActive = true;
                            break;
                        case SDLK_a:
                            inputs.leftActive = true;
                            break;
                        case SDLK_d:
                            inputs.rightActive = true;
                            break;
                        case SDLK_UP:
                            inputs.forwardActive = true;
                            break;
                        case SDLK_DOWN:
                            inputs.backActive = true;
                            break;
                        default:
                            break;
                    }
                    break;
                    
                case SDL_KEYUP:
                    /* Check the SDLKey values and move change the coords */
                    switch( event.key.keysym.sym ) {
                        case SDLK_w:
                            inputs.upActive = false;
                            break;
                        case SDLK_s:
                            inputs.downActive = false;
                            break;
                        case SDLK_a:
                            inputs.leftActive = false;
                            break;
                        case SDLK_d:
                            inputs.rightActive = false;
                            break;
                        case SDLK_UP:
                            inputs.forwardActive = false;
                            break;
                        case SDLK_DOWN:
                            inputs.backActive = false;
                            break;
                        default:
                            break;
                    }
                    break;
                    
                default:
                    break;
            }
        }

        
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - END
        
        accumulator += frameTime;
        
        while ( accumulator >= dt )
        {
            
            // GAME STATE UPDATE - START
            //            integrate( state, t, dt );
            
            if (inputs.leftActive) {
                rollVelocity -= rollAcceleration * dt;
            }
            
            if (inputs.rightActive) {
                rollVelocity += rollAcceleration * dt;
            }
            
            if (!inputs.leftActive && !inputs.rightActive) {
                rollVelocity *= 0.95;
                
                if (fabs(rollVelocity) < 0.05) {
                    rollVelocity = 0.0;
                }
            }
            
            if (rollVelocity < -1.0) {
                rollVelocity = -1.0;
            } else if (rollVelocity > 1.0) {
                rollVelocity = 1.0;
            }
            
            if (inputs.upActive) {
                pitchVelocity -= pitchAcceleration * dt;
            }
            
            if (inputs.downActive) {
                pitchVelocity += pitchAcceleration * dt;
            }
            
            if (!inputs.upActive && !inputs.downActive) {
                pitchVelocity *= 0.95;
            }
            
            if (pitchVelocity < -1.0) {
                pitchVelocity = -1.0;
            } else if (pitchVelocity > 1.0) {
                pitchVelocity = 1.0;
            }
            
            view.roll(rollVelocity * rollMultiplier);
            view.pitch(pitchVelocity * pitchMultiplier);
            
            // GAME STATE UPDATE - END
            
            accumulator -= dt;
            t += dt;
        }
        
        // GAME STATE RENDER - START

        
//        render( state );
        glClearColor ( 0.05, 0.0, 0.1, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        CHECK_GL_ERRORS();
        
        Mat4x4 projectionMatrix;
        Mat4x4 viewMatrix;
        
        buildProjectionMatrix(projectionMatrix, 45.0, 4.0 / 3.0, 10.0, 50000.0);
        
//        double radAngle = degToRad(angle);
//        double cs = cos(radAngle);
//        double sn = sin(radAngle);
//        v3 from(5.05 * cs, 5.05 * sn, 0.2);
//        v3 dir(-from.x, -from.y, -from.z);
//        v3 up(0.0, 0.0, 1.0);
//        
//        from = planetRadius * from;
//        dir = planetRadius * dir;
//
//        view.position = from;
//        view.direction = dir;
//
//        view.updateForUpVector(up);
        
        view.getCameraMatrix(viewMatrix);
        
        multMatrix(projectionMatrix, viewMatrix);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, projectionMatrix);
        CHECK_GL_ERRORS();

        glUseProgram(shaderProgram);
        CHECK_GL_ERRORS();

        posXMesh.draw();
        negXMesh.draw();
        posYMesh.draw();
        negYMesh.draw();
        posZMesh.draw();
        negZMesh.draw();
        
        SDL_GL_SwapWindow(window);
        CHECK_GL_ERRORS();
        
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