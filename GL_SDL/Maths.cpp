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


union _Vector3
{
    struct { float x, y, z; };
    struct { float r, g, b; };
    struct { float s, t, p; };
    float v[3];
    
    _Vector3() {}
    
    _Vector3(double _x, double _y, double _z) {
        x = _x; y = _y; z = _z;
    }
    
    _Vector3(float _x, float _y, float _z) {
        x = _x; y = _y; z = _z;
    }
    
    _Vector3(float _v[3]) {
        x = _v[0]; y = _v[1]; z = _v[2];
    }
    
    _Vector3(double _v[3]) {
        x = _v[0]; y = _v[1]; z = _v[2];
    }
};

typedef union _Vector3 v3;

inline v3 operator + (v3 a, const v3 &b) {
    return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline v3 operator - (v3 a, const v3 &b) {
    return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline v3 operator * (const v3 &a, float s) {
    return v3(a.x *s, a.y * s, a.z * s);
}

inline v3 operator * (float s, const v3 &a) {
    return v3(a.x *s, a.y * s, a.z * s);
}

inline float v3dot(const v3 &a, const v3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline v3 v3cross(const v3 &a, const v3 &b) {
    return v3(
              a.y * b.z - b.y * a.z,
              a.z * b.x - b.z * a.x,
              a.x * b.y - b.x * a.y
              );
}

inline float v3length(const v3 &a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

v3 v3normalize(const v3 &a) {
    
    float length = v3length(a);
    
    return v3(a.x / length, a.y / length, a.z / length);
}

typedef float Mat4x4[16];

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

void setCamera(Mat4x4 &viewMatrix, v3 pos, v3 lookAt) {
    
    v3 dir, up(0.0, 0.0, 1.0);

    dir = lookAt - pos;
//    dir[0] =  (lookAt[0] - pos[0]);
//    dir[1] =  (lookAt[1] - pos[1]);
//    dir[2] =  (lookAt[2] - pos[2]);
    dir = v3normalize(dir);
    
    v3 right = v3cross(dir, up);
    right = v3normalize(right);
    
    up = v3cross(right, dir);
    up = v3normalize(up);
    
    float aux[16];
    
    //    setIdentityMatrix(viewMatrix, 4);
    viewMatrix[0]  = right.x;
    viewMatrix[4]  = right.y;
    viewMatrix[8]  = right.z;
    viewMatrix[12] = 0.0f;
    
    viewMatrix[1]  = up.x;
    viewMatrix[5]  = up.y;
    viewMatrix[9]  = up.z;
    viewMatrix[13] = 0.0f;
    
    viewMatrix[2]  = -dir.x;
    viewMatrix[6]  = -dir.y;
    viewMatrix[10] = -dir.z;
    viewMatrix[14] =  0.0f;
    
    viewMatrix[3]  = 0.0f;
    viewMatrix[7]  = 0.0f;
    viewMatrix[11] = 0.0f;
    viewMatrix[15] = 1.0f;
    
    setTranslationMatrix(aux, -pos.x, -pos.y, -pos.z);
    
    multMatrix(viewMatrix, aux);
}

