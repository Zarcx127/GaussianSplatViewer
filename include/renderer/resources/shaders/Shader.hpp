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

#ifndef RENDERER_RESOURCES_SHADERS_SHADER_H
#define RENDERER_RESOURCES_SHADERS_SHADER_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/ShaderPaths.hpp"

vk::Pipeline make_graphics_pipeline(
    vk::Device device,
    ShaderAsset vertexShader,
    ShaderAsset fragmentShader,
    vk::PipelineLayout pipelineLayout,
    vk::Format colorFormat,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::Pipeline make_compute_pipeline(
    vk::Device device,
    ShaderAsset computeShader,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue  
);

#endif
