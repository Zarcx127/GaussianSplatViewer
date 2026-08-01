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

    uint32_t splatCount { 0 };

    glm::vec3 boundsMin {};
    glm::vec3 boundsMax {};
};

struct SphericalHarmonicBuffer
{
    AllocatedBuffer buffer {};

    uint32_t degree { 0 };
    uint32_t coefficientCount { 0 };
};

#endif
