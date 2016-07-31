#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec4 ex_Color;
//out vec4 gl_FragColor;
out vec4 fragColor;

void main(void) {
    // Pass through our original color with full opacity.
    fragColor = ex_Color;
}
