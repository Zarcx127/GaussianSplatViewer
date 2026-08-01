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

#pragma once

#ifndef RENDERER_SHADER_PATHS_H
#define RENDERER_SHADER_PATHS_H

#include <cstddef>
#include <cstdint>

#ifdef DEBUG

using ShaderAsset = const char*;

namespace shader
{
    constexpr ShaderAsset background =
        "gpu/background/Background.comp.spv";

    constexpr ShaderAsset splatPreprocess =
        "gpu/splats/preprocessing/SplatPreprocess.comp.spv";

    constexpr ShaderAsset splatTile =
        "gpu/splats/tiling/SplatTile.comp.spv";

    constexpr ShaderAsset splatEntryLocalScan =
        "gpu/splats/allocation/SplatEntryLocalScan.comp.spv";

    constexpr ShaderAsset splatEntryBlockScan =
        "gpu/splats/allocation/SplatEntryBlockScan.comp.spv";

    constexpr ShaderAsset splatEntryAddBlockOffsets =
        "gpu/splats/allocation/SplatEntryAddBlockOffsets.comp.spv";

    constexpr ShaderAsset splatEntryFinalize =
        "gpu/splats/allocation/SplatEntryFinalize.comp.spv";

    constexpr ShaderAsset splatSortHistogram = 
        "gpu/splats/sorting/SplatSortHistogram.comp.spv";

    constexpr ShaderAsset splatSortHistogramLocalScan = 
        "gpu/splats/sorting/SplatSortHistogramLocalScan.comp.spv";

    constexpr ShaderAsset splatSortHistogramBlockScan = 
        "gpu/splats/sorting/SplatSortHistogramBlockScan.comp.spv";

    constexpr ShaderAsset splatSortHistogramAddBlockOffsets = 
        "gpu/splats/sorting/SplatSortHistogramAddBlockOffsets.comp.spv";

    constexpr ShaderAsset splatSortBucketOffsetScan =
        "gpu/splats/sorting/SplatSortBucketOffsetScan.comp.spv";
    
    constexpr ShaderAsset splatSortScatter =
        "gpu/splats/sorting/SplatSortScatter.comp.spv";
    
    constexpr ShaderAsset splatTileRange =
        "gpu/splats/ranges/SplatTileRange.comp.spv";

    constexpr ShaderAsset splatTileRender =
        "gpu/splats/rendering/SplatTileRender.comp.spv";

    namespace loading
    {
        constexpr ShaderAsset loadingScreenVertex =
            "gpu/loading/LoadingScreen.vert.spv";

        constexpr ShaderAsset loadingScreenFragment =
            "gpu/loading/LoadingScreen.frag.spv";
    }
}

#else

#include "generated/background/Background.comp.spv.hpp"
#include "generated/splats/preprocessing/SplatPreprocess.comp.spv.hpp"
#include "generated/splats/tiling/SplatTile.comp.spv.hpp"
#include "generated/splats/allocation/SplatEntryLocalScan.comp.spv.hpp"
#include "generated/splats/allocation/SplatEntryBlockScan.comp.spv.hpp"
#include "generated/splats/allocation/SplatEntryAddBlockOffsets.comp.spv.hpp"
#include "generated/splats/allocation/SplatEntryFinalize.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortHistogram.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortHistogramLocalScan.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortHistogramBlockScan.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortHistogramAddBlockOffsets.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortBucketOffsetScan.comp.spv.hpp"
#include "generated/splats/sorting/SplatSortScatter.comp.spv.hpp"
#include "generated/splats/ranges/SplatTileRange.comp.spv.hpp"
#include "generated/splats/rendering/SplatTileRender.comp.spv.hpp"
#include "generated/loading/LoadingScreen.vert.spv.hpp"
#include "generated/loading/LoadingScreen.frag.spv.hpp"

struct ShaderAsset 
{
    const uint32_t* data;
    size_t size;
};

namespace shader
{
    constexpr ShaderAsset background = {
        reinterpret_cast<const uint32_t*>(background_Background_comp_spv),
        background_Background_comp_spv_len
    };

    constexpr ShaderAsset splatPreprocess = {
        reinterpret_cast<const uint32_t*>(splats_preprocessing_SplatPreprocess_comp_spv),
        splats_preprocessing_SplatPreprocess_comp_spv_len
    };

    constexpr ShaderAsset splatTile = {
        reinterpret_cast<const uint32_t*>(splats_tiling_SplatTile_comp_spv),
        splats_tiling_SplatTile_comp_spv_len
    };

    constexpr ShaderAsset splatEntryLocalScan = {
        reinterpret_cast<const uint32_t*>(splats_allocation_SplatEntryLocalScan_comp_spv),
        splats_allocation_SplatEntryLocalScan_comp_spv_len
    };

    constexpr ShaderAsset splatEntryBlockScan = {
        reinterpret_cast<const uint32_t*>(splats_allocation_SplatEntryBlockScan_comp_spv),
        splats_allocation_SplatEntryBlockScan_comp_spv_len
    };

    constexpr ShaderAsset splatEntryAddBlockOffsets = {
        reinterpret_cast<const uint32_t*>(splats_allocation_SplatEntryAddBlockOffsets_comp_spv),
        splats_allocation_SplatEntryAddBlockOffsets_comp_spv_len
    };

    constexpr ShaderAsset splatEntryFinalize = {
        reinterpret_cast<const uint32_t*>(splats_allocation_SplatEntryFinalize_comp_spv),
        splats_allocation_SplatEntryFinalize_comp_spv_len
    };

    constexpr ShaderAsset splatSortHistogram = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortHistogram_comp_spv),
        splats_sorting_SplatSortHistogram_comp_spv_len
    };

    constexpr ShaderAsset splatSortHistogramLocalScan = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortHistogramLocalScan_comp_spv),
        splats_sorting_SplatSortHistogramLocalScan_comp_spv_len
    };

    constexpr ShaderAsset splatSortHistogramBlockScan = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortHistogramBlockScan_comp_spv),
        splats_sorting_SplatSortHistogramBlockScan_comp_spv_len
    };

    constexpr ShaderAsset splatSortHistogramAddBlockOffsets = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortHistogramAddBlockOffsets_comp_spv),
        splats_sorting_SplatSortHistogramAddBlockOffsets_comp_spv_len
    };

    constexpr ShaderAsset splatSortBucketOffsetScan = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortBucketOffsetScan_comp_spv),
        splats_sorting_SplatSortBucketOffsetScan_comp_spv_len
    };

    constexpr ShaderAsset splatSortScatter = {
        reinterpret_cast<const uint32_t*>(splats_sorting_SplatSortScatter_comp_spv),
        splats_sorting_SplatSortScatter_comp_spv_len
    };

    constexpr ShaderAsset splatTileRange = {
        reinterpret_cast<const uint32_t*>(splats_ranges_SplatTileRange_comp_spv),
        splats_ranges_SplatTileRange_comp_spv_len
    };

    constexpr ShaderAsset splatTileRender = {
        reinterpret_cast<const uint32_t*>(splats_rendering_SplatTileRender_comp_spv),
        splats_rendering_SplatTileRender_comp_spv_len
    };

    namespace loading
    {
        constexpr ShaderAsset loadingScreenVertex = {
            reinterpret_cast<const uint32_t*>(loading_LoadingScreen_vert_spv),
            loading_LoadingScreen_vert_spv_len
        };

        constexpr ShaderAsset loadingScreenFragment = {
            reinterpret_cast<const uint32_t*>(loading_LoadingScreen_frag_spv),
            loading_LoadingScreen_frag_spv_len
        };
    }
}

#endif

#endif
