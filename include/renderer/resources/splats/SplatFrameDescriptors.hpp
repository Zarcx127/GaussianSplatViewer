#pragma once

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_DESCRIPTORS_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_DESCRIPTORS_H

#include <deque>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/Splat.hpp"
#include "renderer/resources/splats/SplatFrame.hpp"

enum class SplatFrameBinding : uint32_t
{
    SplatData,
    ProjectedSplats,
    VisibleSplatIndices,
    EntryKeysA,
    EntryKeysB,
    EntrySplatIndicesA,
    EntrySplatIndicesB,
    Counters,
    DrawCommand,
    TileRanges,
    EntryRanges,
    EntryScanBlockSums,
    SortRadixHistograms,
    SortRadixScanBlockSums,
    SortRadixBucketOffsets,
    Count
};

constexpr uint32_t SPLAT_FRAME_BINDING_COUNT =
    static_cast<uint32_t>(SplatFrameBinding::Count);

vk::DescriptorSetLayout build_splat_frame_descriptor_set_layout(
    vk::Device& device,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

bool write_splat_frame_descriptor_set(
    vk::Device device,
    vk::DescriptorSet descriptorSet,
    const SplatBuffer& splatBuffer,
    const SplatFrameResources& resources
);

#endif
