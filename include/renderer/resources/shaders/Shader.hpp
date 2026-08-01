#pragma once

#ifndef RENDERER_RESOURCES_SHADERS_SHADER_H
#define RENDERER_RESOURCES_SHADERS_SHADER_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/pipeline/GraphicsPipelineConfig.hpp"

vk::Pipeline make_compute_pipeline(
    vk::Device device,
    const char* computeFileName,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue  
);

vk::Pipeline make_graphics_pipeline(
    vk::Device device,
    const char* vertexFileName,
    const char* fragmentFileName,
    const GraphicsPipelineConfig& config,
    vk::PipelineLayout pipelineLayout,
    vk::Format colorFormat,
    vk::Format depthFormat,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
