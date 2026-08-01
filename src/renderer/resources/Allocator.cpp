#include "renderer/resources/Allocator.hpp"

#include "logging/Logger.hpp"

VmaAllocator make_vma_allocator(
    vk::Instance instance, 
    vk::PhysicalDevice physicalDevice, 
    vk::Device logicalDevice
) {
    Logger* logger = Logger::get_logger();

    uint32_t version = vk::enumerateInstanceVersion().value;
    version &= ~(0xFFFU);

    VmaAllocatorCreateInfo allocatorInfo = {};
    
    allocatorInfo.instance = instance;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = logicalDevice;
    allocatorInfo.vulkanApiVersion = version;

    VmaAllocator vmaAllocator;
    VkResult vmaAllocatorResult = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
    if(vmaAllocatorResult != VK_SUCCESS)
    {
        logger->print("Failed to create VMA Allocator");
        return nullptr;
    }

    return vmaAllocator;
}
