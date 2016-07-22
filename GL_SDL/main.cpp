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
//#include <GL3/gl3.h>

#include <SDL2/SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

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

void runMainLoop(SDL_Window *mainWindow) {
    
    double r = 0.0;
    
    while (true) {
        
        r += 0.01;
        
        if (r > 1.0) {
            r = 0.0;
        }
        
        glClearColor ( r, 0.0, 0.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        
        SDL_GL_SwapWindow(mainWindow);
    }
}

/* Our program's entry point */
int main(int argc, char *argv[])
{
    SDL_Window *mainWindow; /* Our window handle */
    SDL_GLContext mainContext; /* Our opengl context handle */
    
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
    mainWindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainWindow) /* Die if creation failed */
        sdldie("Unable to create window");
    
    checkSDLError(__LINE__);
    
    /* Create our opengl context and attach it to our window */
    mainContext = SDL_GL_CreateContext(mainWindow);
    checkSDLError(__LINE__);
    
    SDL_GL_SetSwapInterval(1);
    
    runMainLoop(mainWindow);
    
    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    
    /* Clear our buffer with a red background */
    /* Wait 2 seconds */
//    SDL_Delay(2000);
//    
//    /* Same as above, but green */
//    glClearColor ( 0.0, 1.0, 0.0, 1.0 );
//    glClear ( GL_COLOR_BUFFER_BIT );
//    SDL_GL_SwapWindow(mainwindow);
//    SDL_Delay(2000);
//    
//    /* Same as above, but blue */
//    glClearColor ( 0.0, 0.0, 1.0, 1.0 );
//    glClear ( GL_COLOR_BUFFER_BIT );
//    SDL_GL_SwapWindow(mainwindow);
//    SDL_Delay(2000);
    
    /* Delete our opengl context, destroy our window, and shutdown SDL */
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
    
    return 0;
}