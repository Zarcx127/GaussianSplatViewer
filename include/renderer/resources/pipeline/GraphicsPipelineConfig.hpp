#pragma once

#ifndef RENDERER_RESOURCES_PIPELINE_GRAPHICS_PIPELINE_CONFIG_H
#define RENDERER_RESOURCES_PIPELINE_GRAPHICS_PIPELINE_CONFIG_H

#include <vector>

#include <vulkan/vulkan.hpp>

struct GraphicsPipelineConfig
{
    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;

    vk::PrimitiveTopology topology {};

    vk::CullModeFlags cullMode {};
    vk::FrontFace frontFace {};

    bool depthTest { false };
    bool depthWrite { false };

    vk::CompareOp depthCompareOp {};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment {};
};

#endif
