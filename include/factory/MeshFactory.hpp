#pragma once

#ifndef FACTORY_MESH_FACTORY_H
#define FACTORY_MESH_FACTORY_H

#include <array>
#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "renderer/resources/meshes/Mesh.hpp"
#include "renderer/resources/pipeline/GraphicsPipelineConfig.hpp"

vk::VertexInputBindingDescription get_mesh_vertex_binding_description();

std::array<vk::VertexInputAttributeDescription, 2> get_mesh_vertex_attribute_descriptions();

GraphicsPipelineConfig get_mesh_pipeline_config();

Mesh build_cube(
    VmaAllocator& allocator, 
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
);

#endif
