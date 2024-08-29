#version 460

const vec2 positions[] = {
    {  0.0, -0.5 },
    { -0.5, 0.5 },
    {  0.5, 0.5 },
};
const vec3 colors[] = {
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
};

layout (location = 0) out vec3 outColor;

void main(){
    outColor = colors[gl_VertexIndex];
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}