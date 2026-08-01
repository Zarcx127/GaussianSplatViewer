#include "renderer/RenderCommands.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Image.hpp"

#include "renderer/passes/SplatCullPass.hpp"
#include "renderer/passes/BackgroundPass.hpp"
#include "renderer/passes/SplatGaussianPass.hpp"

namespace
{
    void build_rendering_info(
        vk::RenderingInfoKHR& renderingInfo,
        vk::RenderingAttachmentInfoKHR& colorAttachment,
        vk::RenderingAttachmentInfoKHR& depthAttachment,
        const RenderFrameContext& context
    );

    void build_color_attachment(
        vk::RenderingAttachmentInfoKHR& colorAttachment,
        const RenderFrameContext& context
    );

    void build_depth_attachment(
        vk::RenderingAttachmentInfoKHR& depthAttachment,
        const RenderFrameContext& context
    );

    void initialize_render_state(
        vk::CommandBuffer commandBuffer,
        const RenderFrameContext& context
    );
}

bool record_frame_commands(
    vk::CommandBuffer commandBuffer, 
    const RenderFrameContext& context,
    const FramePushConstant& pushConstants
) {
    Logger* logger = Logger::get_logger();

    if(!commandBuffer)
    {
        logger->print("Invalid frame command buffer");
        return false;
    }

    if(
        !context.target.colorImage ||
        !context.target.colorImageView ||
        !context.target.storageDescriptorSet ||
        !context.target.depthImage.image ||
        !context.target.depthImage.imageView ||
        !context.pipelineLayout ||
        !context.splatPointPipeline ||
        !context.splatFrameDescriptorSet ||
        !context.features.backgroundPipeline ||
        !context.features.splatCullPipeline ||
        !context.features.sphericalHarmonicBuffer.buffer.buffer ||
        !context.features.sphericalHarmonicDescriptorSet ||
        !context.features.splatBuffer.buffer.buffer ||
        !context.splatResources.projectedSplats.buffer ||
        !context.splatResources.sortKeys[0].buffer ||
        !context.splatResources.counters.buffer ||
        !context.splatResources.drawCommand.buffer ||
        (context.features.splatBuffer.splatCount == 0)
    ) {
        logger->print("Invalid render frame resource");
        return false;
    }

    if(commandBuffer.reset() != vk::Result::eSuccess)
    {
        logger->print("Failed to reset frame command buffer");
        return false;
    }

    vk::RenderingAttachmentInfoKHR colorAttachment = {};
    vk::RenderingAttachmentInfoKHR depthAttachment = {};
    vk::RenderingInfoKHR renderingInfo = {};

    build_color_attachment(colorAttachment, context);
    build_depth_attachment(depthAttachment, context);
    build_rendering_info(renderingInfo, colorAttachment, depthAttachment, context);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    if(commandBuffer.begin(beginInfo) != vk::Result::eSuccess)
    {
        logger->print("Failed to begin frame command buffer");
        return false;
    }

    transition_image_layout(
        commandBuffer,
        context.target.colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eGeneral,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eShaderWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::ImageAspectFlagBits::eColor
    );

    commandBuffer.pushConstants(
        context.pipelineLayout,
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eVertex 
        ),
        0,
        sizeof(FramePushConstant),
        &pushConstants
    );

    record_background_pass(
        commandBuffer, 
        context.features.backgroundPipeline, 
        context.pipelineLayout, 
        context.target.storageDescriptorSet, 
        context.target.extent
    );

    record_splat_cull_pass(
        commandBuffer,
        context.features.splatCullPipeline,
        context.pipelineLayout,
        context.features.sphericalHarmonicDescriptorSet,
        context.splatFrameDescriptorSet,
        context.splatResources
    );

    transition_image_layout(
        commandBuffer,
        context.target.colorImage,
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );

    initialize_render_state(commandBuffer, context);

    commandBuffer.beginRenderingKHR(renderingInfo);

    record_splat_gaussian_pass(
        commandBuffer, 
        context.splatPointPipeline, 
        context.pipelineLayout,
        context.splatFrameDescriptorSet,
        context.splatResources
    );

    commandBuffer.endRenderingKHR();

    transition_image_layout(
        commandBuffer,
        context.target.colorImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eNone,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    if(commandBuffer.end() != vk::Result::eSuccess)
    {
        logger->print("Failed to end frame command buffer");
        return false;
    }

    return true;
}

namespace
{
    void build_rendering_info(
        vk::RenderingInfoKHR& renderingInfo,
        vk::RenderingAttachmentInfoKHR& colorAttachment,
        vk::RenderingAttachmentInfoKHR& depthAttachment,
        const RenderFrameContext& context
    ) {
        renderingInfo.flags = vk::RenderingFlagsKHR();
        renderingInfo.renderArea = vk::Rect2D({0, 0}, context.target.extent);
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;
        renderingInfo.pStencilAttachment = nullptr;
    }

    void build_color_attachment(
        vk::RenderingAttachmentInfoKHR& colorAttachment,
        const RenderFrameContext& context
    ) {
        colorAttachment.imageView = context.target.colorImageView;
        colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
    }

    void build_depth_attachment(
        vk::RenderingAttachmentInfoKHR& depthAttachment,
        const RenderFrameContext& context
    ) {
        vk::ClearValue clearValue = {};
        clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

        depthAttachment.imageView = context.target.depthImage.imageView;
        depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.clearValue = clearValue;
    }

    void initialize_render_state(
        vk::CommandBuffer commandBuffer,
        const RenderFrameContext& context
    ) {
        vk::Viewport viewport = vk::Viewport(
            0.0f, 
            0.0f, 
            static_cast<float>(context.target.extent.width), 
            static_cast<float>(context.target.extent.height), 
            0.0f, 
            1.0f
        );

        vk::Rect2D scissor = vk::Rect2D({0, 0}, context.target.extent);

        commandBuffer.setViewport(0, 1, &viewport);
        commandBuffer.setScissor(0, 1, &scissor);
    }
}
