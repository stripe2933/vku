#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexcoord;

layout (location = 0) out vec2 fragTexcoord;

layout (push_constant) uniform PushConstant {
    mat4 transform;
} pc;

void main(){
    fragTexcoord = inTexcoord;
    gl_Position = pc.transform * vec4(inPosition, 1.0);
}