#version 450

struct ProjectedSplat
{
    vec4 clipCenter;
    vec4 ellipseAxes;
    vec4 color;
};

layout(set = 2, binding = 1, std430) readonly buffer ProjectedSplatBuffer
{
    ProjectedSplat splats[];
} projectedSplatBuffer;

layout(set = 2, binding = 5, std430) readonly buffer EntrySplatIndexBuffer
{
    uint indices[];
} entrySplatIndexBuffer;

layout(location = 0) out vec2 gaussianPosition;
layout(location = 1) flat out vec4 gaussianColor;

layout(push_constant) uniform PushConstants
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
    uvec4 renderInfo;
} pushConstants;

const float SIGMA_EXTENT = 3.0;

vec2 quad_corner(uint vertexIndex);

void main()
{
    uint splatIndex = entrySplatIndexBuffer.indices[gl_InstanceIndex];
    ProjectedSplat splat = projectedSplatBuffer.splats[splatIndex];

    vec2 corner = quad_corner(gl_VertexIndex);
    vec2 majorAxis = splat.ellipseAxes.xy;
    vec2 minorAxis = splat.ellipseAxes.zw;

    vec2 pixelOffset = (corner.x * majorAxis) + (corner.y * minorAxis);
    vec2 viewport = vec2(pushConstants.renderInfo.xy);
    vec2 ndcOffset = pixelOffset * (2.0 / viewport);

    gl_Position = splat.clipCenter;
    gl_Position.xy += ndcOffset * splat.clipCenter.w;

    gaussianPosition = corner * SIGMA_EXTENT;
    gaussianColor = splat.color;
}

vec2 quad_corner(uint vertexIndex)
{
    const vec2 corners[6] = vec2[6](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0,  1.0)
    );

    return corners[vertexIndex];
}