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

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_H

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

#include "renderer/resources/buffers/Buffer.hpp"

#include "renderer/resources/splats/SplatSort.hpp"

struct alignas(16) SplatTileRangePushConstant
{
    uint32_t sortedBufferIndex { 0 };
};

struct alignas(16) SplatTileRenderPushConstant
{
    uint32_t sortedBufferIndex { 0 };
};

struct alignas(4) GpuProjectedSplat
{
    glm::vec4 clipCenter {};
    glm::vec4 conicOpacity {};
    glm::vec4 color {};
    glm::uvec4 tileBounds {};
};

struct alignas(4) GpuSplatEntryKey
{
    uint32_t tileIndex { 0 };
    uint32_t depthKey { 0 };
};

struct alignas(4) GpuSplatEntryRange
{
    uint32_t entryCount { 0 };
    uint32_t entryOffset { 0 };
};

struct alignas(4) GpuSplatCounters
{
    uint32_t visibleCount { 0 };
    uint32_t entryCount { 0 };
    uint32_t requestedEntryCount { 0 };
    uint32_t overflowCount { 0 };
};

struct alignas(4) GpuSplatTileRange
{
    uint32_t startIndex { 0 };
    uint32_t endIndex { 0 };
};

struct SplatFrameResources
{
    AllocatedBuffer projectedSplats {};
    AllocatedBuffer visibleSplatIndices {};
    AllocatedBuffer entryRanges {};

    std::array<AllocatedBuffer, 2> entryKeys {};
    std::array<AllocatedBuffer, 2> entrySplatIndices {};

    SplatSortResources sort {};

    AllocatedBuffer counters {};
    AllocatedBuffer counterReadback {};
    AllocatedBuffer drawCommand {};
    AllocatedBuffer tileRanges {};
    AllocatedBuffer entryScanBlockSums {};
    
    uint32_t splatCapacity { 0 };
    uint32_t entryCapacity { 0 };
    uint32_t tileCapacity { 0 };
    uint32_t entryScanBlockCapacity { 0 };
};

constexpr uint32_t SPLAT_TILE_SIZE = 16;
constexpr uint32_t SPLAT_ENTRY_SCAN_LOCAL_SIZE = 256;

bool read_splat_frame_counters(
    VmaAllocator allocator,
    const SplatFrameResources& resources,
    GpuSplatCounters& counters    
);

bool splat_frame_resources_are_valid(
    const SplatFrameResources& resources
);

#endif
