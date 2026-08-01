#ifndef SPLAT_HELPERS_GLSL_H
#define SPLAT_HELPERS_GLSL_H

#extension GL_GOOGLE_include_directive : require

#include "SplatConstants.glsl"
#include "SplatStructures.glsl"

uint linear_workgroup_index(uvec3 workGroupID, uvec3 workGroupCount)
{
    return (
        workGroupID.x +
        (workGroupID.y * workGroupCount.x)
    );
}

uint extract_splat_sort_bucket(
    SplatEntryKey key,
    uint keyComponent,
    uint digitShift
) {
    uint keyValue = key.depthKey;

    if(keyComponent == SPLAT_SORT_KEY_TILE)
        keyValue = key.tileIndex;

    return (
        (keyValue >> digitShift) &
        (SPLAT_SORT_RADIX_BUCKET_COUNT - 1)
    );
}

#endif
