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
    SortDispatchCommands,
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
