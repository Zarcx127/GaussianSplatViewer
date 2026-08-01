#include "renderer/resources/buffers/Buffer.hpp"

#include <cstring>

#include "logging/Logger.hpp"

#include "renderer/core/Command.hpp"

bool copy_buffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
) {
    Logger* logger = Logger::get_logger();

    if(!srcBuffer || !dstBuffer || (copySize == 0))
    {
        logger->print("Invalid buffer copy");
        return false;
    }

    return immediate_submit(
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

AllocatedBuffer create_buffer(
    VmaAllocator allocator,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags allocationFlags,
    const char* allocationName
) {
    Logger* logger = Logger::get_logger();

    if(size == 0)
    {
        logger->print("Cannot create zero-sized buffer");
        return {};
    }

    AllocatedBuffer buffer = {};    
    buffer.size = size;

    vk::BufferCreateInfo bufferInfo = {};
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = memoryUsage;
    allocationInfo.flags = allocationFlags;

    VkBuffer rawBuffer = VK_NULL_HANDLE;
    VmaAllocation rawAllocation = nullptr;
    VmaAllocationInfo rawAllocationInfo = {};

    VkBufferCreateInfo rawBufferInfo = bufferInfo;

    VkResult bufferResult = vmaCreateBuffer(
        allocator,
        &rawBufferInfo,
        &allocationInfo,
        &rawBuffer,
        &rawAllocation,
        &rawAllocationInfo
    );

    if(bufferResult != VK_SUCCESS)
    {
        logger->print("Failed to create buffer");
        return {};
    }

    if(allocationName)
        vmaSetAllocationName(allocator, rawAllocation, allocationName);

    vmaGetAllocationInfo(allocator, rawAllocation, &rawAllocationInfo);

    buffer.buffer = rawBuffer;
    buffer.allocation = rawAllocation;
    buffer.allocationInfo = rawAllocationInfo;

    logger->log(buffer.allocationInfo);

    return buffer;
}

bool write_buffer(
    VmaAllocator allocator,
    const AllocatedBuffer& buffer,
    const void* data,
    vk::DeviceSize size
) {
    Logger* logger = Logger::get_logger();

    if(
        !buffer.buffer || !buffer.allocation || !data || 
        (size == 0) || (size > buffer.size)
    ) {
        logger->print("Invalid buffer write");
        return false;
    }

    void* dst = nullptr;
    VkResult mappingResult = vmaMapMemory(allocator, buffer.allocation, &dst);

    if(mappingResult != VK_SUCCESS)
    {
        logger->print("Failed to map buffer memory");
        return false;
    }

    std::memcpy(dst, data, static_cast<size_t>(size));

    if(vmaFlushAllocation(allocator, buffer.allocation, 0, size) != VK_SUCCESS)
    {
        logger->print("Failed to flush buffer memory");
        return false;
    }

    vmaUnmapMemory(allocator, buffer.allocation);

    return true;
}

void destroy_buffer(VmaAllocator allocator, AllocatedBuffer& buffer)
{
    if(buffer.buffer && buffer.allocation)
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);

    buffer = {};
}