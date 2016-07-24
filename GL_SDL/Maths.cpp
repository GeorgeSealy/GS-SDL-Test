//
//  Maths.cpp
//  GL_SDL
//
//  Created by George Sealy on 24/07/16.
//  Copyright © 2016 MixBit. All rights reserved.
//

double degToRad(double inDegrees) {
    return inDegrees * M_PI / 180.0;
}

double radToDeg(double inRadians) {
    return inRadians * 180.0 / M_PI;
}

// ----------------------------------------------------
// VECTOR STUFF
//


typedef float Vec3[3];
typedef float Mat4x4[16];

float dotProduct(Vec3 a, Vec3 b) {
    
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void crossProduct(Vec3 a, Vec3 b, Vec3 &res) {
    
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}

void normalize(Vec3 &a) {
    
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
void setIdentityMatrix(Mat4x4 &mat) {
    
    int size = 4;
    
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
void multMatrix(Mat4x4 &a, Mat4x4 b) {
    
    Mat4x4 res;
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, sizeof(Mat4x4));
    
}

// Defines a transformation matrix mat with a translation
void setTranslationMatrix(Mat4x4 &mat, float x, float y, float z) {
    
    setIdentityMatrix(mat);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}

//float projMatrix[16];
//float viewMatrix[16];

// ----------------------------------------------------
// Projection Matrix
//

void buildProjectionMatrix(Mat4x4 &projMatrix, float fov, float ratio, float nearP, float farP) {
    
    float f = 1.0f / tan (fov * (M_PI / 360.0));
    
    setIdentityMatrix(projMatrix);
    
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

void setCamera(Mat4x4 &viewMatrix, Vec3 pos, Vec3 lookAt) {
    
    Vec3 dir, right, up;
    
    up[0] = 0.0f;   up[1] = 0.0f;   up[2] = 1.0f;
    
    dir[0] =  (lookAt[0] - pos[0]);
    dir[1] =  (lookAt[1] - pos[1]);
    dir[2] =  (lookAt[2] - pos[2]);
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
    
    setTranslationMatrix(aux, -pos[0], -pos[1], -pos[2]);
    
    multMatrix(viewMatrix, aux);
}

