#pragma once

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

VmaAllocator make_vma_allocator(
    vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice
);

#endif
