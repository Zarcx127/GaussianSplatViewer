#include "renderer/resources/buffers/Buffer.hpp"

void copy_buffer(
    vk::Buffer srcBuffer,
    VmaAllocationInfo srcInfo,
    vk::Buffer dstBuffer,
    VmaAllocationInfo dstInfo,
    vk::DeviceSize copySize,
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

    vk::BufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = copySize;

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    (void) commandBuffer.end();

    vk::FenceCreateInfo fenceInfo{};
    vk::Fence fence = device.createFence(fenceInfo).value;

    vk::SubmitInfo submitInfo;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    (void) queue.submit(1, &submitInfo, fence);
    (void) device.waitForFences(1, &fence, vk::True, UINT64_MAX);

    device.destroyFence(fence);
    device.freeCommandBuffers(commandPool, commandBuffer);
}
