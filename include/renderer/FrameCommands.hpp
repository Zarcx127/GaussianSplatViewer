#pragma once

#ifndef RENDERER_FRAME_COMMANDS_H
#define RENDERER_FRAME_COMMANDS_H

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/SplatFrame.hpp"

#include "renderer/RenderFeatures.hpp"

struct alignas(16) FramePushConstant
{
    glm::mat4 view;
    glm::vec4 cameraPosition;
    glm::vec4 projectionInfo;
    glm::uvec4 renderInfo;
};

struct RenderTarget
{
    vk::Image colorImage;
    vk::Extent2D extent;

    vk::DescriptorSet storageDescriptorSet;
};

class FrameCommands
{
public:
    FrameCommands(
        vk::CommandBuffer commandBuffer,
        const RenderTarget& renderTarget,
        const SplatFrameResources& splatResources,
        vk::DescriptorSet splatFrameDescriptorSet,
        vk::PipelineLayout pipelineLayout,
        const RenderFeatureFrameInfo& featureInfo
    );

    FrameCommands(const FrameCommands&) = delete;
    FrameCommands& operator=(const FrameCommands&) = delete;

    bool record(const FramePushConstant& pushConstants);

private:
    vk::CommandBuffer m_commandBuffer {};

    const RenderTarget* m_renderTarget { nullptr };
    const SplatFrameResources* m_splatResources { nullptr };

    vk::DescriptorSet m_splatFrameDescriptorSet {};
    vk::PipelineLayout m_pipelineLayout {};

    const RenderFeatureFrameInfo* m_featureInfo { nullptr };

    bool is_valid() const;
};

bool render_target_is_valid(const RenderTarget& target);

#endif
