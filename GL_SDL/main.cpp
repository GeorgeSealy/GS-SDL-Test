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
    v4 *cols = (v4 *)malloc(sizeof(v4) * numVerts);

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
            
            double multiplier = 0.025;
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
            cols[vertIndex].a = 1.0;
            
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

Mesh buildAtmosphereMesh(int squaresPerSide, v3 startPt, v3 acrossDir, v3 upDir) {
    
    float scaleFactor = 1.0 / squaresPerSide;
    
    v3 uStep = scaleFactor * acrossDir;
    v3 vStep = scaleFactor * upDir;
    
    int numVerts = (squaresPerSide + 1) * (squaresPerSide + 1);
    
    v3 *verts = (v3 *)malloc(sizeof(v3) * numVerts);
    v3 *norms = (v3 *)malloc(sizeof(v3) * numVerts);
    v4 *cols = (v4 *)malloc(sizeof(v4) * numVerts);
    
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
            
            float height = 5.0;
            
            double multiplier = 0.025;
            float distFromCentre = planetRadius * (1.0 + height * multiplier);
            
            pt = distFromCentre * pt;
            cols[vertIndex].r = 0.7;
            cols[vertIndex].g = 0.75;
            cols[vertIndex].b = 0.9;
            cols[vertIndex].a = 0.3;
            
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

struct Model {
    static const int maxMeshes = 64;
    
    int numMeshes;
    Mesh meshes[maxMeshes];
    
    void render() {
        for (int i = 0; i < numMeshes; ++i) {
            meshes[i].draw();
        }
    }
};

Model planet;
Model atmosphere;

void setupGL() {
    
    int numSquaresPerSide = 128;
    
    planet.numMeshes = 6;
//    planet.meshes[0] = buildGridMesh(numSquaresPerSide, v3(1.0, -1.0, -1.0), v3(0.0, 2.0, 0.0), v3(0.0, 0.0, 2.0));
//    planet.meshes[1] = buildGridMesh(numSquaresPerSide, v3(-1.0, 1.0, -1.0), v3(0.0, -2.0, 0.0), v3(0.0, 0.0, 2.0));
//    planet.meshes[2] = buildGridMesh(numSquaresPerSide, v3(1.0, 1.0, -1.0), v3(-2.0, 0.0, 0.0), v3(0.0, 0.0, 2.0));
//    planet.meshes[3] = buildGridMesh(numSquaresPerSide, v3(-1.0, -1.0, -1.0), v3(2.0, 0.0, 0.0), v3(0.0, 0.0, 2.0));
//    planet.meshes[4] = buildGridMesh(numSquaresPerSide, v3(-1.0, -1.0, 1.0), v3(2.0, 0.0, 0.0), v3(0.0, 2.0, 0.0));
//    planet.meshes[5] = buildGridMesh(numSquaresPerSide, v3(-1.0, -1.0, -1.0), v3(2.0, 0.0, 0.0), v3(0.0, 2.0, 0.0));

    planet.meshes[0] = buildGridMesh(numSquaresPerSide, v3(1.0, -1.0, 1.0), v3(0.0, 2.0, 0.0), v3(0.0, 0.0, -2.0));
    planet.meshes[1] = buildGridMesh(numSquaresPerSide, v3(-1.0, 1.0, 1.0), v3(0.0, -2.0, 0.0), v3(0.0, 0.0, -2.0));
    planet.meshes[2] = buildGridMesh(numSquaresPerSide, v3(1.0, 1.0, 1.0), v3(-2.0, 0.0, 0.0), v3(0.0, 0.0, -2.0));
    planet.meshes[3] = buildGridMesh(numSquaresPerSide, v3(-1.0, -1.0, 1.0), v3(2.0, 0.0, 0.0), v3(0.0, 0.0, -2.0));
    planet.meshes[4] = buildGridMesh(numSquaresPerSide, v3(-1.0, 1.0, 1.0), v3(2.0, 0.0, 0.0), v3(0.0, -2.0, 0.0));
    planet.meshes[5] = buildGridMesh(numSquaresPerSide, v3(-1.0, -1.0, -1.0), v3(2.0, 0.0, 0.0), v3(0.0, 2.0, 0.0));
    
    numSquaresPerSide = 64;
    atmosphere.numMeshes = 6;
    atmosphere.meshes[0] = buildAtmosphereMesh(numSquaresPerSide, v3(1.0, -1.0, 1.0), v3(0.0, 2.0, 0.0), v3(0.0, 0.0, -2.0));
    atmosphere.meshes[1] = buildAtmosphereMesh(numSquaresPerSide, v3(-1.0, 1.0, 1.0), v3(0.0, -2.0, 0.0), v3(0.0, 0.0, -2.0));
    atmosphere.meshes[2] = buildAtmosphereMesh(numSquaresPerSide, v3(1.0, 1.0, 1.0), v3(-2.0, 0.0, 0.0), v3(0.0, 0.0, -2.0));
    atmosphere.meshes[3] = buildAtmosphereMesh(numSquaresPerSide, v3(-1.0, -1.0, 1.0), v3(2.0, 0.0, 0.0), v3(0.0, 0.0, -2.0));
    atmosphere.meshes[4] = buildAtmosphereMesh(numSquaresPerSide, v3(-1.0, 1.0, 1.0), v3(2.0, 0.0, 0.0), v3(0.0, -2.0, 0.0));
    atmosphere.meshes[5] = buildAtmosphereMesh(numSquaresPerSide, v3(-1.0, -1.0, -1.0), v3(2.0, 0.0, 0.0), v3(0.0, 2.0, 0.0));


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
    
    void yaw(double angle) {
        up = v3normalize(up);
        
        direction = rotatePointAboutLine(direction, angle, v3(0.0, 0.0, 0.0), up);
        
        updateForUpVector(up);
    }
    
    void move(double distance) {
        direction = v3normalize(direction);

        v3 offset = distance * direction;
        
        position = position + offset;
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
        yawLeftActive = false;
        yawRightActive = false;
        upActive = false;
        downActive = false;
        shouldQuit = false;
    }
    
    bool forwardActive;
    bool backActive;
    bool leftActive;
    bool rightActive;
    bool yawLeftActive;
    bool yawRightActive;
    bool upActive;
    bool downActive;
    bool shouldQuit;
};

InputState updateInputs(InputState &inputs) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                inputs.shouldQuit = true;
                break;
                
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
                    case SDLK_LEFT:
                        inputs.yawLeftActive = true;
                        break;
                    case SDLK_RIGHT:
                        inputs.yawRightActive = true;
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
                    case SDLK_LEFT:
                        inputs.yawLeftActive = false;
                        break;
                    case SDLK_RIGHT:
                        inputs.yawRightActive = false;
                        break;
                    default:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
    
    return inputs;
}

struct Ship {
    double rollAcceleration;
    double rollVelocity;
    double rollMultiplier;
    
    double pitchAcceleration;
    double pitchVelocity;
    double pitchMultiplier;
    
    double yawAcceleration;
    double yawVelocity;
    double yawMultiplier;
    
    double acceleration;
    double velocity;
    double velocityMultiplier;
    
    PointOfView view;
    
    Ship(const PointOfView &_view) {
        
        view = _view;
        
        rollAcceleration = 0.5;
        rollVelocity = 0.0;
        rollMultiplier = 0.3;
        
        pitchAcceleration = 0.1;
        pitchVelocity = 0.0;
        pitchMultiplier = 0.2;
        
        yawAcceleration = 0.1;
        yawVelocity = 0.0;
        yawMultiplier = 0.2;
        
        acceleration = 0.1;
        velocity = 0.0;
        velocityMultiplier = 1000.0;
    }
    
    void moveShip(const InputState &inputs, double dt, v3 planetPos, double planetRad) {
        
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
            pitchVelocity *= 0.9;
        }
        
        if (pitchVelocity < -1.0) {
            pitchVelocity = -1.0;
        } else if (pitchVelocity > 1.0) {
            pitchVelocity = 1.0;
        }
        
        if (inputs.yawLeftActive) {
            yawVelocity += yawAcceleration * dt;
        }
        
        if (inputs.yawRightActive) {
            yawVelocity -= yawAcceleration * dt;
        }
        
        if (!inputs.yawLeftActive && !inputs.yawRightActive) {
            yawVelocity *= 0.9;
        }
        
        if (yawVelocity < -1.0) {
            yawVelocity = -1.0;
        } else if (yawVelocity > 1.0) {
            yawVelocity = 1.0;
        }
        
        if (inputs.forwardActive) {
            velocity += acceleration * dt;
        }
        
        if (inputs.backActive) {
            velocity -= acceleration * dt;
        }
        
        if (!inputs.forwardActive && !inputs.backActive) {
            velocity *= 0.95;
        }
        
        if (velocity < -1.0) {
            velocity = -1.0;
        } else if (velocity > 1.0) {
            velocity = 1.0;
        }
        
        view.roll(rollVelocity * rollMultiplier);
        view.pitch(pitchVelocity * pitchMultiplier);
        view.yaw(yawVelocity * yawMultiplier);
        
        velocityMultiplier = v3length(view.position - planetPos) - planetRad;
        
        if (velocityMultiplier < 10.0) {
            velocityMultiplier = 10.0;
        }
        
        view.move(velocity * dt * velocityMultiplier);
    }
};

void runMainLoop(SDL_Window *window) {
    
    setupGL();
    
    double t = 0.0;
    const double dt = 1.0 / 60.0;
    
    double currentTime = timer.seconds();
    double accumulator = 0.0;

    PointOfView view;
    
    v3 from(5.0, 0.0, 0.2);
    v3 dir(-from.x, -from.y, -from.z);
    v3 up(0.0, 0.0, 1.0);
    
    from = planetRadius * from;
    view.position = from;
    view.direction = dir;
    view.updateForUpVector(up);

    Ship ship(view);
    
    InputState inputs;
    
    // Basic run loop from http://gafferongames.com/game-physics/fix-your-timestep/
    // TODO: (George) Final interpolation between states for super smoothness
    
    while (!inputs.shouldQuit)
    {
        double newTime = timer.seconds();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - START
        inputs = updateInputs(inputs);
        
        // GATHER USER INPUT (PLUS NETWORK INPUT?) - END
        
        accumulator += frameTime;
        
        while ( accumulator >= dt )
        {
            
            // GAME STATE UPDATE - START
            //            integrate( state, t, dt );
            
            ship.moveShip(inputs, dt, v3(0.0, 0.0, 0.0), planetRadius);
            
            // GAME STATE UPDATE - END
            
            accumulator -= dt;
            t += dt;
        }
        
        // GAME STATE RENDER - START

        
        glClearColor ( 0.05, 0.0, 0.1, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        CHECK_GL_ERRORS();
        
        Mat4x4 projectionMatrix;
        Mat4x4 viewMatrix;
        
        buildProjectionMatrix(projectionMatrix, 45.0, 4.0 / 3.0, 10.0, 50000.0);
        
        ship.view.getCameraMatrix(viewMatrix);
        
        multMatrix(projectionMatrix, viewMatrix);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpmatrix"), 1, GL_FALSE, projectionMatrix);
        CHECK_GL_ERRORS();

        glUseProgram(shaderProgram);
        CHECK_GL_ERRORS();

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        planet.render();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        
        glCullFace(GL_FRONT);
        atmosphere.render();
        
        // TODO: (George) Only if we're outside the atmosphere...
        glCullFace(GL_BACK);
        atmosphere.render();
        
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