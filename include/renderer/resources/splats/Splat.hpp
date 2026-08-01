#pragma once

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_H

#include <cstdint>

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/buffers/Buffer.hpp"

struct GpuSplat
{
    glm::vec4 position {};
    glm::vec4 color {};
    glm::vec4 logScale {};
    glm::vec4 rotation {};
};

struct SplatBuffer
{
    AllocatedBuffer buffer {};
    vk::DeviceSize bufferOffset { 0 };

    uint32_t splatCount { 0 };

    glm::vec3 boundsMin {};
    glm::vec3 boundsMax {};
};

#endif
