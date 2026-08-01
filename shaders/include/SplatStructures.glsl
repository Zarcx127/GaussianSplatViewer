#ifndef SPLAT_STRUCTURES_GLSL
#define SPLAT_STRUCTURES_GLSL

struct Splat
{
    vec4 position;
    vec4 color;
    vec4 logScale;
    vec4 rotation;
};

struct SortDispatchCommand
{
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
    uint valueCount;
};

struct ProjectedSplat
{
    vec4 clipCenter;
    vec4 conicOpacity;
    vec4 color;
    uvec4 tileBounds;
};

struct SplatEntryKey
{
    uint tileIndex;
    uint depthKey;
};

struct SplatEntryRange
{
    uint entryCount;
    uint entryOffset;
};

struct SplatTileRange
{
    uint startIndex;
    uint endIndex;
};

#endif
