#pragma once

#ifndef RENDERER_SHADER_PATHS_H
#define RENDERER_SHADER_PATHS_H

namespace shader
{
    constexpr const char* background =
        "gpu/background-Background.comp.spv";

    constexpr const char* splatPreprocess =
        "gpu/splats-preprocessing-SplatPreprocess.comp.spv";

    constexpr const char* splatTile =
        "gpu/splats-tiling-SplatTile.comp.spv";

    constexpr const char* splatEntryLocalScan =
        "gpu/splats-allocation-SplatEntryLocalScan.comp.spv";

    constexpr const char* splatEntryBlockScan =
        "gpu/splats-allocation-SplatEntryBlockScan.comp.spv";

    constexpr const char* splatEntryAddBlockOffsets =
        "gpu/splats-allocation-SplatEntryAddBlockOffsets.comp.spv";

    constexpr const char* splatEntryFinalize =
        "gpu/splats-allocation-SplatEntryFinalize.comp.spv";

    constexpr const char* splatSortHistogram = 
        "gpu/splats-sorting-SplatSortHistogram.comp.spv";

    constexpr const char* splatSortHistogramLocalScan = 
        "gpu/splats-sorting-SplatSortHistogramLocalScan.comp.spv";

    constexpr const char* splatSortHistogramBlockScan = 
        "gpu/splats-sorting-SplatSortHistogramBlockScan.comp.spv";

    constexpr const char* splatSortHistogramAddBlockOffsets = 
        "gpu/splats-sorting-SplatSortHistogramAddBlockOffsets.comp.spv";

    constexpr const char* splatSortBucketOffsetScan =
        "gpu/splats-sorting-SplatSortBucketOffsetScan.comp.spv";
    
    constexpr const char* splatSortScatter =
        "gpu/splats-sorting-SplatSortScatter.comp.spv";
    
    constexpr const char* splatTileRange =
        "gpu/splats-ranges-SplatTileRange.comp.spv";

    constexpr const char* splatTileRender =
        "gpu/splats-rendering-SplatTileRender.comp.spv";
}

#endif
