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

#ifndef RENDERER_CORE_DEVICE_H
#define RENDERER_CORE_DEVICE_H

#include <cstdint>
#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

struct PhysicalDeviceSelection
{
    vk::PhysicalDevice device;
    uint32_t queueFamilyIndex { UINT32_MAX };
};

PhysicalDeviceSelection choose_physical_device(
    vk::Instance instance, 
    vk::SurfaceKHR surface
);

uint32_t find_queue_family_index(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    vk::QueueFlags queueType
);

vk::Device create_logical_device(
    vk::PhysicalDevice physicalDevice, 
    uint32_t queueFamilyIndex,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
