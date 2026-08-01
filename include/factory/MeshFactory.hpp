#pragma once

#ifndef MESH_FACTORY_H
#define MESH_FACTORY_H

#include <deque>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

struct Mesh
{
    vk::Buffer buffer;
    vk::DeviceSize offset;

    VmaAllocation allocation;

    uint32_t numOfVertices;
};

vk::VertexInputBindingDescription2EXT get_binding_description();

std::vector<vk::VertexInputAttributeDescription2EXT> get_attribute_descriptions();

Mesh build_triangle(
    VmaAllocator& allocator, 
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

#endif
