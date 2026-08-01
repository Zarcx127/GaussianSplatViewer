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

#ifndef SHADER_BINDINGS_GLSL
#define SHADER_BINDINGS_GLSL

#extension GL_GOOGLE_include_directive : require

#include "SplatStructures.glsl"

#ifdef FRAME_COLOR_IMAGE_ACCESS
layout(set = 0, binding = 0, rgba8) 
uniform FRAME_COLOR_IMAGE_ACCESS image2D colorBuffer;
#endif

#ifdef SPHERICAL_HARMONIC_BUFFER_ACCESS
layout(set = 1, binding = 0, std430) 
SPHERICAL_HARMONIC_BUFFER_ACCESS buffer SphericalHarmonicBuffer
{
    vec4 coefficients[];
}
sphericalHarmonicBuffer;
#endif

#ifdef SPLAT_DATA_BUFFER_ACCESS
layout(set = 2, binding = 0, std430) 
SPLAT_DATA_BUFFER_ACCESS buffer SplatBuffer
{
    Splat splats[];
}
splatBuffer;
#endif

#ifdef PROJECTED_SPLAT_BUFFER_ACCESS
layout(set = 2, binding = 1, std430) 
PROJECTED_SPLAT_BUFFER_ACCESS buffer ProjectedSplatBuffer
{
    ProjectedSplat splats[];
}
projectedSplatBuffer;
#endif

#ifdef VISIBLE_SPLAT_INDEX_BUFFER_ACCESS
layout(set = 2, binding = 2, std430) 
VISIBLE_SPLAT_INDEX_BUFFER_ACCESS buffer VisibleSplatIndexBuffer
{
    uint indices[];
}
visibleSplatIndexBuffer;
#endif

#ifdef ENTRY_KEY_BUFFER_A_ACCESS
layout(set = 2, binding = 3, std430) 
ENTRY_KEY_BUFFER_A_ACCESS buffer SplatEntryKeyBufferA
{
    SplatEntryKey keys[];
}
entryKeyBufferA;
#endif

#ifdef ENTRY_KEY_BUFFER_B_ACCESS
layout(set = 2, binding = 4, std430) 
ENTRY_KEY_BUFFER_B_ACCESS buffer SplatEntryKeyBufferB
{
    SplatEntryKey keys[];
}
entryKeyBufferB;
#endif

#ifdef ENTRY_SPLAT_INDEX_BUFFER_A_ACCESS
layout(set = 2, binding = 5, std430) 
ENTRY_SPLAT_INDEX_BUFFER_A_ACCESS buffer EntrySplatIndexBufferA
{
    uint indices[];
}
entrySplatIndexBufferA;
#endif

#ifdef ENTRY_SPLAT_INDEX_BUFFER_B_ACCESS

layout(set = 2, binding = 6, std430) 
ENTRY_SPLAT_INDEX_BUFFER_B_ACCESS buffer EntrySplatIndexBufferB
{
    uint indices[];
}
entrySplatIndexBufferB;

#endif

#ifdef SPLAT_COUNTER_BUFFER_ACCESS
layout(set = 2, binding = 7, std430) 
SPLAT_COUNTER_BUFFER_ACCESS buffer SplatCounterBuffer
{
    uint visibleCount;
    uint entryCount;
    uint requestedEntryCount;
    uint overflowCount;
}
counters;
#endif

#ifdef SPLAT_DRAW_COMMAND_BUFFER_ACCESS
layout(set = 2, binding = 8, std430) 
SPLAT_DRAW_COMMAND_BUFFER_ACCESS buffer SplatDrawCommandBuffer
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
}
drawCommand;
#endif

#ifdef SPLAT_TILE_RANGE_BUFFER_ACCESS
layout(set = 2, binding = 9, std430) 
SPLAT_TILE_RANGE_BUFFER_ACCESS buffer SplatTileRangeBuffer
{
    SplatTileRange ranges[];
}
tileRangeBuffer;
#endif

#ifdef SPLAT_ENTRY_RANGE_BUFFER_ACCESS
layout(set = 2, binding = 10, std430) 
SPLAT_ENTRY_RANGE_BUFFER_ACCESS buffer SplatEntryRangeBuffer
{
    SplatEntryRange ranges[];
}
splatEntryRangeBuffer;
#endif

#ifdef ENTRY_SCAN_BLOCK_SUM_BUFFER_ACCESS
layout(set = 2, binding = 11, std430) 
ENTRY_SCAN_BLOCK_SUM_BUFFER_ACCESS buffer EntryScanBlockSumBuffer
{
    uint values[];
}
entryScanBlockSumBuffer;
#endif

#ifdef RADIX_HISTOGRAM_BUFFER_ACCESS
layout(set = 2, binding = 12, std430) 
RADIX_HISTOGRAM_BUFFER_ACCESS buffer RadixHistogramBuffer
{
    uint values[];
}
radixHistogramBuffer;
#endif

#ifdef RADIX_SCAN_BLOCK_SUM_BUFFER_ACCESS
layout(set = 2, binding = 13, std430) 
RADIX_SCAN_BLOCK_SUM_BUFFER_ACCESS buffer RadixScanBlockSumBuffer
{
    uint values[];
}
radixScanBlockSumBuffer;
#endif

#ifdef RADIX_BUCKET_OFFSET_BUFFER_ACCESS
layout(set = 2, binding = 14, std430) 
RADIX_BUCKET_OFFSET_BUFFER_ACCESS buffer RadixBucketOffsetBuffer
{
    uint values[];
}
radixBucketOffsetBuffer;
#endif

#ifdef SPLAT_SORT_DISPATCH_BUFFER_ACCESS
layout(set = 2, binding = 15, std430) 
SPLAT_SORT_DISPATCH_BUFFER_ACCESS buffer SplatSortDispatchBuffer
{
    SortDispatchCommand entries;
    SortDispatchCommand histogramBlocks;
}
sortDispatchBuffer;
#endif

#if defined(ENTRY_KEY_BUFFER_A_ACCESS) && \
    defined(ENTRY_KEY_BUFFER_B_ACCESS)

SplatEntryKey read_splat_entry_key(uint entryIndex, uint bufferIndex) 
{
    if(bufferIndex == 0)
        return entryKeyBufferA.keys[entryIndex];

    return entryKeyBufferB.keys[entryIndex];
}

#endif

#if defined(ENTRY_SPLAT_INDEX_BUFFER_A_ACCESS) && \
    defined(ENTRY_SPLAT_INDEX_BUFFER_B_ACCESS)

uint read_entry_splat_index(uint entryIndex, uint bufferIndex) 
{
    if(bufferIndex == 0)
        return entrySplatIndexBufferA.indices[entryIndex];

    return entrySplatIndexBufferB.indices[entryIndex];
}

#endif

#endif
