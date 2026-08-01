#pragma once

#ifndef FACTORY_SPLAT_FRAME_FACTORY_H
#define FACTORY_SPLAT_FRAME_FACTORY_H

#include <deque>
#include <functional>

#include <vma/vk_mem_alloc.h>

#include "renderer/resources/splats/SplatFrame.hpp"

SplatFrameResources build_splat_frame_resources(
    VmaAllocator allocator,
    uint32_t splatCapacity,
    uint32_t entryCapacity,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

#endif
