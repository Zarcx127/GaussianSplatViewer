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

#ifndef RENDERER_PASSES_SPLAT_TILE_PASS_H
#define RENDERER_PASSES_SPLAT_TILE_PASS_H

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/SplatFrame.hpp"

void record_splat_tile_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
);

#endif
