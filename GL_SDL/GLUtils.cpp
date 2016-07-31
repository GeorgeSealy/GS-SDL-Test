//
//  GLUtils.cpp
//  GL_SDL
//
//  Created by George Sealy on 24/07/16.
//  Copyright © 2016 MixBit. All rights reserved.
//

#include <assert.h>

#ifdef DEBUG

#define CHECK_GL_ERRORS()                               \
do {                                                    \
GLenum error = glGetError();                            \
if(error != GL_NO_ERROR) {                          \
printf("OpenGL: %s [error %d]\n", __FUNCTION__, (int)error);					\
assert(0); \
} \
} while(false)

#else

#define CHECK_GL_ERRORS()

#endif

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
        return 0;
    }
    CHECK_GL_ERRORS();
    
    glShaderSource(shader, 1, (const GLchar**)&shaderSource, 0);
    CHECK_GL_ERRORS();
    glCompileShader(shader);
    CHECK_GL_ERRORS();
    
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
    CHECK_GL_ERRORS();
    //    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    CHECK_GL_ERRORS();
    
    // TODO: (George) Handle this in a more re-usable way
    glBindAttribLocation(shaderProgram, 0, "in_Position");
    CHECK_GL_ERRORS();
    glBindAttribLocation(shaderProgram, 1, "in_Normal");
    CHECK_GL_ERRORS();
    glBindAttribLocation(shaderProgram, 2, "in_Color");
    CHECK_GL_ERRORS();
    
    glLinkProgram(shaderProgram);
    CHECK_GL_ERRORS();

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