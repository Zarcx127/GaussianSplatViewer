#include "renderer/core/Command.hpp"

#ifdef COMMAND_H

#include "logging/Logger.hpp"

vk::CommandPool make_command_pool(
    vk::Device logicalDevice, 
    uint32_t graphicsQueueFamilyIndex, 
    std::deque<std::function<void(vk::Device)>>& deviceDeletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::CommandPoolCreateInfo poolInfo = {};
    poolInfo.setQueueFamilyIndex(graphicsQueueFamilyIndex);
    poolInfo.setFlags(
        vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer
    );

    vk::ResultValue<vk::CommandPool> poolAttempt = logicalDevice.createCommandPool(poolInfo);
    if(poolAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create command pool");
        return vk::CommandPool();
    }

    vk::CommandPool pool = poolAttempt.value;
    deviceDeletionQueue.push_back([logger, pool] (vk::Device device)->void{
        device.destroyCommandPool(pool);
        logger->print("Deleted command pool");
    });

    return pool;
}

vk::CommandBuffer allocate_command_buffer(vk::Device logicalDevice, vk::CommandPool commandPool)
{
    vk::CommandBufferAllocateInfo allocInfo = {};

    allocInfo.setCommandBufferCount(1);
    allocInfo.setCommandPool(commandPool);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

    return logicalDevice.allocateCommandBuffers(allocInfo).value[0];
}

#endif
