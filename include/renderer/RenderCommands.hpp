#pragma once

#ifndef RENDERER_RENDER_COMMANDS_H
#define RENDERER_RENDER_COMMANDS_H

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/images/AllocatedImage.hpp"

#include "renderer/RenderFeatures.hpp"

struct FramePushConstant
{
    glm::mat4 mvp;
    glm::mat4 invView;
    glm::mat4 invProj;
    glm::vec4 cameraPos;
};

struct RenderTarget
{
    vk::Image colorImage;
    vk::ImageView colorImageView;
    vk::Extent2D extent;

    vk::DescriptorSet storageDescriptorSet;

    const AllocatedImage& depthImage;
};

struct RenderFrameContext
{
    const RenderTarget& target;

    const vk::Pipeline& meshPipeline;

    const vk::PipelineLayout& pipelineLayout;

    RenderFeatureFrameInfo features;
};

bool record_frame_commands(
    vk::CommandBuffer commandBuffer,
    const RenderFrameContext& context,
    const FramePushConstant& pushConstants
);

#endif
