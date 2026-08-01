#pragma once

#ifndef RENDERER_FRAME_COMMANDS_H
#define RENDERER_FRAME_COMMANDS_H

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/images/AllocatedImage.hpp"

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
    vk::ImageView colorImageView;
    vk::Extent2D extent;

    vk::DescriptorSet storageDescriptorSet;

    const AllocatedImage& depthImage;
};

class FrameCommands
{
public:
    FrameCommands(
        vk::CommandBuffer commandBuffer,
        const RenderTarget& renderTarget,
        const SplatFrameResources& splatResources,
        vk::DescriptorSet splatFrameDescriptorSet,
        vk::Pipeline splatGaussianPipeline,
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
    vk::Pipeline m_splatGaussianPipeline {};
    vk::PipelineLayout m_pipelineLayout {};

    const RenderFeatureFrameInfo* m_featureInfo { nullptr };

    bool is_valid() const;

    void build_rendering_info(
        vk::RenderingInfoKHR& renderingInfo,
        vk::RenderingAttachmentInfoKHR& colorAttachment,
        vk::RenderingAttachmentInfoKHR& depthAttachment
    ) const;

    void build_color_attachment(
        vk::RenderingAttachmentInfoKHR& colorAttachment
    ) const;

    void build_depth_attachment(
        vk::RenderingAttachmentInfoKHR& depthAttachment
    ) const;

    void initialize_render_state();
};

bool render_target_is_valid(
    const RenderTarget& target
);

#endif
