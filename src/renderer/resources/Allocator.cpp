#include "renderer/resources/Allocator.hpp"

#ifdef ALLOCATOR_H

VmaAllocator make_vma_allocator(
    vk::Instance instance, 
    vk::PhysicalDevice physicalDevice, 
    vk::Device logicalDevice
) {
    uint32_t version = vk::enumerateInstanceVersion().value;
    version &= ~(0xFFFU);

    VmaAllocatorCreateInfo allocatorInfo = {};
    
    allocatorInfo.instance = instance;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = logicalDevice;
    allocatorInfo.vulkanApiVersion = version;

    VmaAllocator vmaAllocator;
    vmaCreateAllocator(&allocatorInfo, &vmaAllocator);

    return vmaAllocator;
}

#endif
