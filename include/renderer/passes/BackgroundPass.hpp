#pragma once

#ifndef RENDERER_PASSES_BACKGROUND_PASS_H
#define RENDERER_PASSES_BACKGROUND_PASS_H

#include <vulkan/vulkan.hpp>

void record_background_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet descriptorSet,
    vk::Extent2D extent
);

#endif
