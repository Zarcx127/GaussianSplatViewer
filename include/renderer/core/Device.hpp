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
