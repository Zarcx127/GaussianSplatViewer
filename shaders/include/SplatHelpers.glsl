/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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
