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

struct Timer {
    double ticksToSeconds;
    
    uint64_t startTick;
    
    Timer() {
        mach_timebase_info_data_t timebase_info;
        mach_timebase_info(&timebase_info);

//        const uint64_t NANOS_PER_MSEC = 1000000ULL;
        ticksToSeconds = ((double)timebase_info.denom / (double)timebase_info.numer) / 1000000000.0;
        
        startTick = mach_absolute_time();
    }
    
    double seconds() {
        uint64_t nowTick = mach_absolute_time();
        return (nowTick - startTick) * ticksToSeconds;
    }
};


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

char *fileToCharArray(const char *file)
{
    FILE *fptr;
    long length;
    char *buf;
    
    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
    
    return buf; /* Return the buffer */
}

typedef enum {
    ShaderTypeVertex,
    ShaderTypeFragment,
    ShaderTypeGeometry,
} ShaderType;

GLuint compileShader(const char *shaderFilename, ShaderType shaderType) {
    GLchar *shaderSource;
    GLuint shader;

    shaderSource = fileToCharArray(shaderFilename);

    switch (shaderType) {
        case ShaderTypeVertex:
            shader = glCreateShader(GL_VERTEX_SHADER);
            break;
            
        case ShaderTypeFragment:
            shader = glCreateShader(GL_FRAGMENT_SHADER);
            break;
            
        case ShaderTypeGeometry:
            shader = glCreateShader(GL_GEOMETRY_SHADER);
            break;
            
        default:
            
            sdldie("Unhandled shader type in compileShader()");
            return 0;
    }

    glShaderSource(shader, 1, (const GLchar**)&shaderSource, 0);
    glCompileShader(shader);
    
    free(shaderSource);
    
    int isShaderCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isShaderCompiled);
    
    if (isShaderCompiled == FALSE) {
        int maxLength;
        
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        char *infoLog = (char *)malloc(maxLength);
        
        glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

        printf("Error compiling shader: %s\n", shaderFilename);
        printf("Error: %s\n", infoLog);
        free(infoLog);
        return 0;
    }
    
    return shader;
}

GLuint linkShaders(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader) {
    GLuint shaderProgram = glCreateProgram();
    
    /* Attach our shaders to our program */
    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    
    // TODO: (George) Handle this in a more re-usable way
    glBindAttribLocation(shaderProgram, 0, "in_Position");
    glBindAttribLocation(shaderProgram, 1, "in_Color");
    
    glLinkProgram(shaderProgram);

    int isLinked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == FALSE) {
        int maxLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        
        char *infoLog = (char *)malloc(maxLength);
        
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, infoLog);
        
        free(infoLog);
        
        return 0;
    }
    
    return shaderProgram;
}

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

const GLubyte indices[6] = { 0, 1, 2, 3, 0, 1 };

void setupGL() {

    vertexShader = compileShader("Assets/Shaders/SimpleCameraVertex.glsl", ShaderTypeVertex);
//    geometryShader = compileShader("Assets/Shaders/SimpleCameraGeometry.glsl", ShaderTypeGeometry);
    fragmentShader = compileShader("Assets/Shaders/SimpleCameraFragment.glsl", ShaderTypeFragment);
    
    printf("Shaders: %u, %u, %u\n", vertexShader, geometryShader, fragmentShader);
    
    shaderProgram = linkShaders(vertexShader, geometryShader, fragmentShader);
    printf("Program: %u\n", shaderProgram);
    
    glUseProgram(shaderProgram);
    
    glGenVertexArrays(1, &vao);
    printf("VAO: %u\n", vao);
    
    glBindVertexArray(vao);
    glGenBuffers(3, vbo);
    printf("VBOs: %u, %u, %u\n", vbo[0], vbo[1], vbo[2]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW);

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

float projMatrix[16];
float viewMatrix[16];

double degToRad(double inDegrees) {
    return inDegrees * M_PI / 180.0;
}

// ----------------------------------------------------
// VECTOR STUFF
//

// res = a cross b;
void crossProduct( float *a, float *b, float *res) {
    
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}

// Normalize a vec3
void normalize(float *a) {
    
    float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);
    
    a[0] /= mag;
    a[1] /= mag;
    a[2] /= mag;
}

// ----------------------------------------------------
// MATRIX STUFF
//

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void setIdentityMatrix( float *mat, int size) {
    
    // fill matrix with 0s
    for (int i = 0; i < size * size; ++i)
        mat[i] = 0.0f;
    
    // fill diagonal with 1s
    for (int i = 0; i < size; ++i)
        mat[i + i * size] = 1.0f;
}

//
// a = a * b;
//
void multMatrix(float *a, float *b) {
    
    float res[16];
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, 16 * sizeof(float));
    
}

// Defines a transformation matrix mat with a translation
void setTranslationMatrix(float *mat, float x, float y, float z) {
    
    setIdentityMatrix(mat,4);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}

// ----------------------------------------------------
// Projection Matrix
//

void buildProjectionMatrix(float fov, float ratio, float nearP, float farP) {
    
    float f = 1.0f / tan (fov * (M_PI / 360.0));
    
    setIdentityMatrix(projMatrix,4);
    
    projMatrix[0] = f / ratio;
    projMatrix[1 * 4 + 1] = f;
    projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
    projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
    projMatrix[2 * 4 + 3] = -1.0f;
    projMatrix[3 * 4 + 3] = 0.0f;
}

// ----------------------------------------------------
// View Matrix
//
// note: it assumes the camera is not tilted,
// i.e. a vertical up vector (remmeber gluLookAt?)
//

void setCamera(float posX, float posY, float posZ,
               float lookAtX, float lookAtY, float lookAtZ) {
    
    float dir[3], right[3], up[3];
    
    up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;
    
    dir[0] =  (lookAtX - posX);
    dir[1] =  (lookAtY - posY);
    dir[2] =  (lookAtZ - posZ);
    normalize(dir);
    
    crossProduct(dir,up,right);
    normalize(right);
    
    crossProduct(right,dir,up);
    normalize(up);
    
    float aux[16];
    
//    setIdentityMatrix(viewMatrix, 4);
    viewMatrix[0]  = right[0];
    viewMatrix[4]  = right[1];
    viewMatrix[8]  = right[2];
    viewMatrix[12] = 0.0f;
    
    viewMatrix[1]  = up[0];
    viewMatrix[5]  = up[1];
    viewMatrix[9]  = up[2];
    viewMatrix[13] = 0.0f;
    
    viewMatrix[2]  = -dir[0];
    viewMatrix[6]  = -dir[1];
    viewMatrix[10] = -dir[2];
    viewMatrix[14] =  0.0f;
    
    viewMatrix[3]  = 0.0f;
    viewMatrix[7]  = 0.0f;
    viewMatrix[11] = 0.0f;
    viewMatrix[15] = 1.0f;
    
    setTranslationMatrix(aux, -posX, -posY, -posZ);
    
    multMatrix(viewMatrix, aux);
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
//        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_BYTE, 0);
        
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