#include "renderer/resources/buffers/Buffer.hpp"

#include "renderer/core/Command.hpp"

void copy_buffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
) {
    immediate_submit(
        device,
        commandPool,
        queue,
        [srcBuffer, dstBuffer, copySize] (vk::CommandBuffer commandBuffer)->void {
            vk::BufferCopy copyRegion = {};
            
            copyRegion.srcOffset = 0; 
            copyRegion.dstOffset = 0; 
            copyRegion.size = copySize;

            commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion); 
        }
    );
}
