#version 450

layout (location = 0) in vec2 fragTexcoord;

layout (location = 0) out vec4 outColor;

layout (early_fragment_tests) in;

void main(){
    outColor = vec4(fragTexcoord, fragTexcoord.x * fragTexcoord.y, 1.0);
}