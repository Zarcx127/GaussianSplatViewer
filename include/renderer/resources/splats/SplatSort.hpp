#pragma once

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_SORT_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_SORT_H

#include <cstdint>

#include "renderer/resources/buffers/Buffer.hpp"

constexpr uint32_t SPLAT_SORT_RADIX_BITS = 4;
constexpr uint32_t SPLAT_SORT_RADIX_BUCKET_COUNT = (
    1U << SPLAT_SORT_RADIX_BITS
);

constexpr uint32_t SPLAT_SORT_KEY_BIT_COUNT = 32;

constexpr uint32_t SPLAT_SORT_LOCAL_SIZE = 256;
constexpr uint32_t SPLAT_SORT_SCAN_LOCAL_SIZE = 256;
constexpr uint32_t SPLAT_SORT_DISPATCH_ROW_CAPACITY = 65535;

enum class SplatSortKeyComponent : uint32_t
{
    Depth,
    Tile
};

struct alignas(16) SplatSortPushConstant
{
    uint32_t digitShift { 0 };
    uint32_t keyComponent { 0 };
    uint32_t inputBufferIndex { 0 };
    uint32_t outputBufferIndex { 1 };
};

struct alignas(16) SplatSortScanPushConstant
{
    uint32_t valueCount { 0 };
    uint32_t inputOffset { 0 };
    uint32_t outputOffset { 0 };
    uint32_t writeBucketTotal { 0 };
};

enum class SplatSortScanTarget : uint32_t
{
    Histograms,
    BlockSums
};

struct alignas(16) SplatSortAddOffsetsPushConstant
{
    uint32_t valueCount { 0 };
    uint32_t targetOffset { 0 };
    uint32_t blockOffset { 0 };
    uint32_t targetBuffer { 0 };
};

struct SplatSortResources
{
    AllocatedBuffer radixHistograms {};
    AllocatedBuffer radixScanBlockSums {};
    AllocatedBuffer radixBucketOffsets {};

    uint32_t workgroupCapacity { 0 };
    uint32_t histogramCapacity { 0 };
    uint32_t histogramBlockCapacity { 0 };
    uint32_t scanBlockCapacityPerBucket { 0 };
};

bool splat_sort_resources_are_valid(const SplatSortResources& resources);

#endif
