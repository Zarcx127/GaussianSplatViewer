#pragma once

#ifndef COMMAND_H
#define COMMAND_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::CommandPool make_command_pool(
    vk::Device device, 
    uint32_t graphicsQueueFamilyIndex, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::CommandBuffer make_command_buffer(
    vk::Device device, 
    vk::CommandPool commandPool,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

void immediate_submit(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    const std::function<void(vk::CommandBuffer)>& function
);

#endif
