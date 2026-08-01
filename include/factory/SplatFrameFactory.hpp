#pragma once

#ifndef FACTORY_SPLAT_FRAME_FACTORY_H
#define FACTORY_SPLAT_FRAME_FACTORY_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

#include "renderer/resources/splats/SplatFrame.hpp"

SplatFrameResources build_splat_frame_resources(
    VmaAllocator allocator,
    uint32_t splatCapacity,
    uint32_t entryCapacity,
    uint32_t tileCapacity,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

uint32_t calculate_splat_entry_capacity(
    vk::PhysicalDevice physicalDevice,
    uint32_t splatCapacity,
    uint32_t framesInFlight
);

uint32_t calculate_grown_splat_entry_capacity(
    vk::PhysicalDevice physicalDevice,
    uint32_t currentCapacity,
    uint32_t requiredCapacity
);

#endif
