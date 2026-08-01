#include "renderer/core/Command.hpp"

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
        return {};
    }

    deletionQueue.push_back(
        [logger, commandPool, commandBuffer] (vk::Device device)->void{
            device.freeCommandBuffers(commandPool, 1, &commandBuffer);
            logger->print("Deleted command buffer");
        }
    );

    return commandBuffer;
}

void immediate_submit(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    const std::function<void(vk::CommandBuffer)>& function
) {
    vk::CommandBuffer commandBuffer = allocate_command_buffer(device, commandPool);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    (void) commandBuffer.begin(beginInfo);
    function(commandBuffer);
    (void) commandBuffer.end();

    vk::FenceCreateInfo fenceInfo = {};
    vk::Fence fence = device.createFence(fenceInfo).value;

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    (void) queue.submit(1, &submitInfo, fence);
    (void) device.waitForFences(1, &fence, vk::True, UINT64_MAX);

    device.destroyFence(fence);
    device.freeCommandBuffers(commandPool, commandBuffer);
}

namespace 
{
    vk::CommandBuffer allocate_command_buffer(vk::Device device, vk::CommandPool commandPool) 
    {
        vk::CommandBufferAllocateInfo allocInfo = {};

        allocInfo.setCommandBufferCount(1);
        allocInfo.setCommandPool(commandPool);
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

        return device.allocateCommandBuffers(allocInfo).value[0];
    }
}
