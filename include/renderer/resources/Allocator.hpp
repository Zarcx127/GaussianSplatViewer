#pragma once

#ifndef RENDERER_RESOURCES_ALLOCATOR_H
#define RENDERER_RESOURCES_ALLOCATOR_H

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

VmaAllocator make_vma_allocator(
    vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice
);

#endif
