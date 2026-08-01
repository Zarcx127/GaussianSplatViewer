#pragma once

#ifndef COMMAND_H
#define COMMAND_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::CommandPool make_command_pool(
    vk::Device logicalDevice, 
    uint32_t graphicsQueueFamilyIndex, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::CommandBuffer allocate_command_buffer(vk::Device logicalDevice, vk::CommandPool commandPool);

#endif
