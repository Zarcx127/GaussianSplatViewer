#pragma once

#ifndef DEVICE_H
#define DEVICE_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

bool supports(const vk::PhysicalDevice& device, std::vector<const char*> requestedExtensions);

bool is_suitable(const vk::PhysicalDevice device);

vk::PhysicalDevice choose_physical_device(const vk::Instance instance);

uint32_t findQueueFamilyIndex(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    vk::QueueFlags queueType
);

vk::Device create_logical_device(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
