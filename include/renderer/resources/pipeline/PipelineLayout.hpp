#pragma once

#ifndef RENDERER_RESOURCES_PIPELINE_PIPELINE_LAYOUT_H
#define RENDERER_RESOURCES_PIPELINE_PIPELINE_LAYOUT_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/shaders/ShaderInterface.hpp"

class PipelineLayoutBuilder
{
public:
    PipelineLayoutBuilder(vk::Device& device);

    vk::PipelineLayout build(
        const ShaderInterface& shaderInterface,
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

private:
    vk::Device* m_device;
};

#endif
