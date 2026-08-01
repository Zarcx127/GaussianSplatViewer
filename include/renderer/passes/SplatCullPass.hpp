#pragma once

#ifndef RENDERER_PASSES_SPLAT_CULL_PASS_H
#define RENDERER_PASSES_SPLAT_CULL_PASS_H

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/SplatFrame.hpp"

void record_splat_cull_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet sphericalHarmonicDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
);

#endif
