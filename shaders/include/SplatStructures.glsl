#ifndef SPLAT_STRUCTURES_GLSL
#define SPLAT_STRUCTURES_GLSL

struct ProjectedSplat
{
    vec4 clipCenter;
    vec4 ellipseAxes;
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