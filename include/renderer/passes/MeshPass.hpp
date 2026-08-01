#pragma once

#ifndef RENDERER_PASSES_MESH_PASS_H
#define RENDERER_PASSES_MESH_PASS_H

#include <vulkan/vulkan.hpp>

#include "renderer/resources/meshes/Mesh.hpp"

void record_mesh_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    const Mesh& mesh
);

#endif
