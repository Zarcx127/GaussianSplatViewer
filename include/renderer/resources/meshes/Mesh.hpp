#pragma once

#ifndef RENDERER_RESOURCES_MESHES_MESH_H
#define RENDERER_RESOURCES_MESHES_MESH_H

#include <cstdint>

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/buffers/Buffer.hpp"

struct MeshVertex
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct Mesh
{
    AllocatedBuffer vertexBuffer {};
    vk::DeviceSize vertexBufferOffset { 0 };

    uint32_t vertexCount { 0 };
};

#endif
