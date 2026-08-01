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

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_SORT_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_SORT_H

#include <cstdint>

#include "renderer/resources/buffers/Buffer.hpp"

constexpr uint32_t SPLAT_SORT_RADIX_BITS = 4;
constexpr uint32_t SPLAT_SORT_RADIX_BUCKET_COUNT = (1U << SPLAT_SORT_RADIX_BITS);

constexpr uint32_t SPLAT_SORT_KEY_BIT_COUNT = 32;

constexpr uint32_t SPLAT_SORT_LOCAL_SIZE = 256;
constexpr uint32_t SPLAT_SORT_SCAN_LOCAL_SIZE = 256;

constexpr vk::DeviceSize SPLAT_SORT_ENTRY_DISPATCH_OFFSET = 0;

struct alignas(4) SplatSortPushConstant
{
    uint32_t digitShift { 0 };
    uint32_t keyComponent { 0 };
    uint32_t inputBufferIndex { 0 };
    uint32_t outputBufferIndex { 1 };
};

struct alignas(4) SplatSortScanPushConstant
{
    uint32_t valueCount { 0 };
    uint32_t inputOffset { 0 };
    uint32_t outputOffset { 0 };
    uint32_t writeBucketTotal { 0 };
};

struct alignas(4) SplatSortAddOffsetsPushConstant
{
    uint32_t valueCount { 0 };
    uint32_t targetOffset { 0 };
    uint32_t blockOffset { 0 };
    uint32_t targetBuffer { 0 };
};

struct alignas(4) GpuSplatSortDispatchCommand
{
    uint32_t groupCountX { 0 };
    uint32_t groupCountY { 1 };
    uint32_t groupCountZ { 1 };
    uint32_t valueCount { 0 };
};

enum class SplatSortKeyComponent : uint32_t
{
    Depth,
    Tile
};

enum class SplatSortScanTarget : uint32_t
{
    Histograms,
    BlockSums
};

struct GpuSplatSortDispatch
{
    GpuSplatSortDispatchCommand entries {};
    GpuSplatSortDispatchCommand histogramBlocks {};
};

struct SplatSortResources
{
    AllocatedBuffer radixHistograms {};
    AllocatedBuffer radixScanBlockSums {};
    AllocatedBuffer radixBucketOffsets {};
    AllocatedBuffer dispatchCommands {};

    uint32_t workgroupCapacity { 0 };
    uint32_t histogramCapacity { 0 };
    uint32_t histogramBlockCapacity { 0 };
    uint32_t scanBlockCapacityPerBucket { 0 };
};

bool splat_sort_resources_are_valid(const SplatSortResources& resources);

#endif
