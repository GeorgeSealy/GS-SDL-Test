//
//  Mesh.cpp
//  GL_SDL
//
//  Created by George Sealy on 24/07/16.
//  Copyright © 2016 MixBit. All rights reserved.
//

struct Mesh {
    GLuint vao = 0;
    GLuint vertexBuffer[3] = {0, 0, 0};
    GLuint indexBuffer = 0;
    int numVertices = 0;
    int numIndices = 0;
    
    void setup(int numVerts, v3 *verts, v3 *norms, v4 *colors, int numInds, ushort *indices) {
        
        if (vao == 0) {
            printf("Mesh setup\n");
            glGenVertexArrays(1, &vao);
            CHECK_GL_ERRORS();
            printf("VAO: %u\n", vao);
            
            glGenBuffers(3, vertexBuffer);
            CHECK_GL_ERRORS();
            printf("VBOs: %u, %u, %u\n", vertexBuffer[0], vertexBuffer[1], vertexBuffer[2]);
            
            glGenBuffers(1, &indexBuffer);
            CHECK_GL_ERRORS();
            printf("IBO: %u\n", indexBuffer);
        }
        
        glBindVertexArray(vao);
        CHECK_GL_ERRORS();
        
        numVertices = numVerts;
        numIndices = numInds;
        
//        printf("Verts:\n");
//        for (int i = 0; i < numVertices; ++i) {
//            printf("  (%1.2f, %1.2f, %1.2f)\n", verts[i].x, verts[i].y, verts[i].z);
//        }
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(v3), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(v3), verts, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(v4), colors, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);
        CHECK_GL_ERRORS();
    }
    
    void draw() {
        glBindVertexArray(vao);
        CHECK_GL_ERRORS();
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
        glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
        CHECK_GL_ERRORS();
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        CHECK_GL_ERRORS();
        
        glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_SHORT, 0);
        CHECK_GL_ERRORS();
    }
};
