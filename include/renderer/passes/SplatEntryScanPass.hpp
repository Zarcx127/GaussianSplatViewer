#pragma once

#ifndef RENDERER_PASSES_SPLAT_ENTRY_SCAN_PASS_H
#define RENDERER_PASSES_SPLAT_ENTRY_SCAN_PASS_H

#include <vulkan/vulkan.hpp>

#include "renderer/resources/pipeline/RenderFeaturePipelines.hpp"

#include "renderer/resources/splats/SplatFrame.hpp"

void record_splat_entry_scan_pass(
    vk::CommandBuffer commandBuffer,
    const SplatEntryScanPipelines& pipelines,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
);

#endif
