#pragma once

#ifndef RENDERER_PASSES_SPLAT_SORT_PASS_H
#define RENDERER_PASSES_SPLAT_SORT_PASS_H

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/pipeline/RenderFeaturePipelines.hpp"

#include "renderer/resources/splats/SplatFrame.hpp"

uint32_t record_splat_sort_pass(
    vk::CommandBuffer commandBuffer,
    const SplatSortPipelines& pipelines,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
);

#endif
