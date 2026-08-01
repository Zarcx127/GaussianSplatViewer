#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/shaders/ShaderInterface.hpp"

std::vector<vk::ShaderEXT> make_shader_object(
    vk::Device device, 
    const char* vertexFileName, 
    const char* fragmentFileName, 
    const ShaderInterface& shaderInterface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::ShaderEXT make_compute_shader(
    vk::Device device, 
    const char* computeFileName, 
    const ShaderInterface& shaderInterface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
