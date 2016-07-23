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
#include <OpenGL/gl.h>

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
            
        default:
            
            sdldie("Unhandled shader type in compileShader()");
            return 0;
    }

    glShaderSource(shader, 1, (const GLchar**)&shaderSource, 0);
    
    /* Compile the vertex shader */
    glCompileShader(shader);
    
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

GLuint vertexShader = 0;
GLuint fragmentShader = 0;

void setupGL() {

    vertexShader = compileShader("Assets/Shaders/SimpleVert.glsl", ShaderTypeVertex);
    fragmentShader = compileShader("Assets/Shaders/SimpleFrag.glsl", ShaderTypeFragment);
    
    printf("Shaders: %u, %u\n", vertexShader, fragmentShader);
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
            
            if (r > 1.0) {
                r = 0.0;
            }
            
            // GAME STATE UPDATE - END
            
            accumulator -= dt;
            t += dt;
        }
        
        // GAME STATE RENDER - START

        
//        render( state );
        glClearColor ( r, 0.0, 0.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        
        SDL_GL_SwapWindow(window);
        
        // GAME STATE RENDER - END
    }
    
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