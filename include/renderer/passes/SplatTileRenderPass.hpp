#pragma once

#ifndef RENDERER_PASSES_SPLAT_TILE_RENDER_PASS_H
#define RENDERER_PASSES_SPLAT_TILE_RENDER_PASS_H

#include <cstdint>

#include <vulkan/vulkan.hpp>

void record_splat_tile_render_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet renderTargetDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    vk::Extent2D extent,
    uint32_t sortedBufferIndex
);

#endif
