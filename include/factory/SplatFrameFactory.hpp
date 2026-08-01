/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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
