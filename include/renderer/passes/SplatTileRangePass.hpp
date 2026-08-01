#pragma once

#ifndef RENDERER_PASSES_SPLAT_TILE_RANGE_PASS_H
#define RENDERER_PASSES_SPLAT_TILE_RANGE_PASS_H

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/SplatFrame.hpp"

void record_splat_tile_range_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources,
    uint32_t sortedBufferIndex
);

#endif
