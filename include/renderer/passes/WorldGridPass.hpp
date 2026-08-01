#pragma once

#ifndef RENDERER_PASSES_WORLD_GRID_PASS_H
#define RENDERER_PASSES_WORLD_GRID_PASS_H

#include <vulkan/vulkan.hpp>

void record_world_grid_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet descriptorSet,
    vk::Extent2D extent
);

#endif
