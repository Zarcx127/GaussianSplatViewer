#pragma once

#ifndef RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_H
#define RENDERER_RESOURCES_SPLATS_SPLAT_FRAME_H

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

#include "renderer/resources/buffers/Buffer.hpp"

struct GpuProjectedSplat
{
    glm::vec4 clipCenter {};
    glm::vec4 ellipseAxes {};
    glm::vec4 color {};
};

struct GpuSplatSortKey
{
    uint32_t tileIndex { 0 };
    uint32_t depthKey { 0 };
};

struct GpuSplatCounters
{
    uint32_t visibleCount { 0 };
    uint32_t entryCount { 0 };
    uint32_t overflowCount { 0 };
    uint32_t padding { 0 };
};

struct GpuSplatDrawCommand
{
    uint32_t vertexCount { 6 };
    uint32_t instanceCount { 0 };
    uint32_t firstVertex { 0 };
    uint32_t firstInstance { 0 };
};

struct SplatFrameResources
{
    AllocatedBuffer projectedSplats {};
    AllocatedBuffer visibleSplatIndices {};

    std::array<AllocatedBuffer, 2> sortKeys {};
    std::array<AllocatedBuffer, 2> entrySplatIndices {};

    AllocatedBuffer counters {};
    AllocatedBuffer drawCommand {};

    uint32_t splatCapacity { 0 };
    uint32_t entryCapacity { 0 };
};

#endif
