#version 450

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstants
{
    mat4 mvp;
    mat4 invView;
    mat4 invProj;
    vec4  cameraPos;
} pc;

void main()
{
    gl_Position = pc.mvp * vec4(vertexPos, 1.0);
    fragColor = vertexColor;
}
