#pragma once

#ifndef RENDERER_SHADER_PATHS_H
#define RENDERER_SHADER_PATHS_H

namespace shader
{
    constexpr const char* background =
        "gpu/compute/background/Background.comp.spv";

    constexpr const char* splatPreprocess =
        "gpu/compute/splats/preprocessing/SplatPreprocess.comp.spv";

    constexpr const char* splatTile =
        "gpu/compute/splats/tiling/SplatTile.comp.spv";

    constexpr const char* splatEntryLocalScan =
        "gpu/compute/splats/allocation/SplatEntryLocalScan.comp.spv";

    constexpr const char* splatEntryBlockScan =
        "gpu/compute/splats/allocation/SplatEntryBlockScan.comp.spv";

    constexpr const char* splatEntryAddBlockOffsets =
        "gpu/compute/splats/allocation/SplatEntryAddBlockOffsets.comp.spv";

    constexpr const char* splatEntryFinalize =
        "gpu/compute/splats/allocation/SplatEntryFinalize.comp.spv";

    constexpr const char* splatSortHistogram = 
        "gpu/compute/splats/sorting/SplatSortHistogram.comp.spv";

    constexpr const char* splatSortHistogramLocalScan = 
        "gpu/compute/splats/sorting/SplatSortHistogramLocalScan.comp.spv";

    constexpr const char* splatSortHistogramBlockScan = 
        "gpu/compute/splats/sorting/SplatSortHistogramBlockScan.comp.spv";

    constexpr const char* splatSortHistogramAddBlockOffsets = 
        "gpu/compute/splats/sorting/SplatSortHistogramAddBlockOffsets.comp.spv";

    constexpr const char* splatSortBucketOffsetScan =
        "gpu/compute/splats/sorting/SplatSortBucketOffsetScan.comp.spv";
    
    constexpr const char* splatSortScatter =
        "gpu/compute/splats/sorting/SplatSortScatter.comp.spv";
    
    constexpr const char* splatTileRange =
        "gpu/compute/splats/ranges/SplatTileRange.comp.spv";

    constexpr const char* splatTileRender =
        "gpu/compute/splats/rendering/SplatTileRender.comp.spv";

    constexpr const char* splatGaussianVertex =
        "gpu/raster/SplatGaussian.vert.spv";

    constexpr const char* splatGaussianFragment =
        "gpu/raster/SplatGaussian.frag.spv";
}

#endif
