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
