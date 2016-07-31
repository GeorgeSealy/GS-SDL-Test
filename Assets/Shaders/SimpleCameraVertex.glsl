#version 150
uniform mat4 mvpmatrix;

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in  vec3 in_Position;
in  vec3 in_Normal;
in  vec4 in_Color;

out vec4 ex_Color;

void main(void) {
    // Since we are using flat lines, our input only had two points: x and y.
    // Set the Z coordinate to 0 and W coordinate to 1
    
    gl_Position = mvpmatrix * vec4(in_Position, 1.0);
//    gl_Position = vec4(in_Position.x, in_Position.y, 0.0, 1.0);
    
    // GLSL allows shorthand use of vectors too, the following is also valid:
    // gl_Position = vec4(in_Position, 0.0, 1.0);
    // We're simply passing the color through unmodified
    
    ex_Color = vec4(in_Color.r * in_Color.a, in_Color.g * in_Color.a, in_Color.b * in_Color.a, in_Color.a);
}
