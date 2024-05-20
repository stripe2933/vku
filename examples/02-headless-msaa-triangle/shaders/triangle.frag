#version 450

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

layout (early_fragment_tests) in;

void main(){
    outColor = vec4(fragColor, 1.0);
}