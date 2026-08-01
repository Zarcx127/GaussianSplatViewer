#pragma once

#ifndef RENDERER_PASSES_SPLAT_POINT_PASS_H
#define RENDERER_PASSES_SPLAT_POINT_PASS_H

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/Splat.hpp"

void record_splat_point_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    const SplatBuffer& splatBuffer
);

#endif
