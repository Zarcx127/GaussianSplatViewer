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

#include "renderer/core/Command.hpp"

#include <vector>

#include "logging/Logger.hpp"

namespace
{
    vk::CommandBuffer allocate_command_buffer(vk::Device device, vk::CommandPool commandPool);
}

vk::CommandPool make_command_pool(
    vk::Device device, 
    uint32_t graphicsQueueFamilyIndex, 
    std::deque<std::function<void(vk::Device)>>& deviceDeletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setQueueFamilyIndex(graphicsQueueFamilyIndex);
    poolInfo.setFlags(
        vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer
    );

    vk::ResultValue<vk::CommandPool> poolAttempt = device.createCommandPool(poolInfo);
    if(poolAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create command pool");
        return vk::CommandPool();
    }

    vk::CommandPool pool = poolAttempt.value;
    deviceDeletionQueue.push_back(
        [logger, pool] (vk::Device device)->void {
            device.destroyCommandPool(pool);
            logger->print("Deleted command pool");
        }
    );

    return pool;
}

vk::CommandBuffer make_command_buffer(
    vk::Device device, 
    vk::CommandPool commandPool,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::CommandBuffer commandBuffer = allocate_command_buffer(device, commandPool);
    if(!commandBuffer)
    {
        logger->print("Failed to allocate command buffer");
        return vk::CommandBuffer();
    }

    deletionQueue.push_back(
        [logger, commandPool, commandBuffer] (vk::Device device)->void{
            device.freeCommandBuffers(commandPool, 1, &commandBuffer);
            logger->print("Deleted command buffer");
        }
    );

    return commandBuffer;
}

bool immediate_submit(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    const std::function<void(vk::CommandBuffer)>& function
) {
    Logger* logger = Logger::get_logger();

    vk::CommandBuffer commandBuffer = allocate_command_buffer(device, commandPool);
    if(!commandBuffer)
    {
        logger->print("Failed to allocate immediate command buffer");
        return false;
    }

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    if(commandBuffer.begin(beginInfo) != vk::Result::eSuccess)
    {
        device.freeCommandBuffers(commandPool, commandBuffer);
        logger->print("Failed to start immediate command buffer");

        return false;
    }

    function(commandBuffer);

    if(commandBuffer.end() != vk::Result::eSuccess)
    {
        device.freeCommandBuffers(commandPool, commandBuffer);
        logger->print("Failed to end immediate command buffer");

        return false;
    }

    vk::FenceCreateInfo fenceInfo = {};
    vk::ResultValue<vk::Fence> fenceAttempt = device.createFence(fenceInfo);
    if(fenceAttempt.result != vk::Result::eSuccess)
    {
        device.freeCommandBuffers(commandPool, commandBuffer);
        logger->print("Failed to create immediate submit fence");

        return false;
    }

    vk::Fence fence = fenceAttempt.value;

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if(queue.submit(1, &submitInfo, fence) != vk::Result::eSuccess)
    {
        device.destroyFence(fence);
        device.freeCommandBuffers(commandPool, commandBuffer);
        logger->print("Failed to submit immediate command buffer");

        return false;
    }
    
    if(device.waitForFences(1, &fence, vk::True, UINT64_MAX) != vk::Result::eSuccess)
    {
        device.destroyFence(fence);
        device.freeCommandBuffers(commandPool, commandBuffer);
        logger->print("Failed to wait for immediate submit fence");
    
        return false;
    }

    device.destroyFence(fence);
    device.freeCommandBuffers(commandPool, commandBuffer);

    return true;
}

namespace 
{
    vk::CommandBuffer allocate_command_buffer(vk::Device device, vk::CommandPool commandPool) 
    {
        vk::CommandBufferAllocateInfo allocInfo = {};

        allocInfo.setCommandBufferCount(1);
        allocInfo.setCommandPool(commandPool);
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        vk::ResultValue<std::vector<vk::CommandBuffer>> commandBufferAttempt =
            device.allocateCommandBuffers(allocInfo);
        
        if(commandBufferAttempt.result != vk::Result::eSuccess)
            return vk::CommandBuffer();

        if(commandBufferAttempt.value.empty())
            return vk::CommandBuffer();

        return commandBufferAttempt.value[0];
    }
}
