#include "renderer/resources/Buffer.hpp"

#ifdef BUFFER_H

void copyBuffer(
    vk::Buffer srcBuffer,
    VmaAllocationInfo srcInfo,
    vk::Buffer dstBuffer,
    VmaAllocationInfo dstInfo,
    uint32_t copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
) {
    vk::CommandBufferAllocateInfo allocInfo = {};

    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocInfo).value[0];
    
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    (void) commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion;

    copyRegion.srcOffset = srcInfo.offset;
    copyRegion.dstOffset = dstInfo.offset;
    copyRegion.size = copySize;

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    (void) commandBuffer.end();

    vk::FenceCreateInfo fenceInfo{};
    vk::Fence fence = device.createFence(fenceInfo).value;

    vk::SubmitInfo submitInfo;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    (void) queue.submit(1, &submitInfo, fence);
    (void) device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);

    device.destroyFence(fence);
    device.freeCommandBuffers(commandPool, commandBuffer);
}

#endif
