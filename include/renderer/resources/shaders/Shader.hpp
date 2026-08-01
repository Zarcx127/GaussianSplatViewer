#pragma once

#ifndef RENDERER_RESOURCES_SHADERS_SHADER_H
#define RENDERER_RESOURCES_SHADERS_SHADER_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::Pipeline make_compute_pipeline(
    vk::Device device,
    const char* computeFileName,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue  
);

#endif
