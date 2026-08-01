#pragma once

#ifndef FACTORY_SPLAT_FACTORY_H
#define FACTORY_SPLAT_FACTORY_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "assets/splats/SplatCloud.hpp"

#include "renderer/resources/pipeline/GraphicsPipelineConfig.hpp"

#include "renderer/resources/splats/Splat.hpp"

GraphicsPipelineConfig get_splat_gaussian_pipeline_config();

SplatBuffer build_splat_buffer(
    const SplatCloud& cloud,
    VmaAllocator& allocator,
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

SphericalHarmonicBuffer build_spherical_harmonic_buffer(
    const SplatCloud& cloud,
    VmaAllocator& allocator,
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

#endif
