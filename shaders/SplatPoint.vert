#version 450

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec4 vertexColor;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants
{
    mat4 mvp;
    mat4 invView;
    mat4 invProj;
    vec4 cameraPos;
} pushConstants;

void main()
{
    gl_Position = pushConstants.mvp * vec4(vertexPos, 1.0);
    gl_PointSize = 1.0;

    fragColor = vertexColor;
}
