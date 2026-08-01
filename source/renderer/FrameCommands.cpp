#include "renderer/FrameCommands.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Image.hpp"

#include "renderer/core/Synchronization.hpp"

#include "renderer/passes/SplatPreprocessPass.hpp"
#include "renderer/passes/BackgroundPass.hpp"
#include "renderer/passes/SplatGaussianPass.hpp"
#include "renderer/passes/SplatTilePass.hpp"
#include "renderer/passes/SplatEntryScanPass.hpp"
#include "renderer/passes/SplatSortPass.hpp"
#include "renderer/passes/SplatTileRangePass.hpp"
#include "renderer/passes/SplatTileRenderPass.hpp"

FrameCommands::FrameCommands(
    vk::CommandBuffer commandBuffer,
    const RenderTarget& renderTarget,
    const SplatFrameResources& splatResources,
    vk::DescriptorSet splatFrameDescriptorSet,
    vk::Pipeline splatGaussianPipeline,
    vk::PipelineLayout pipelineLayout,
    const RenderFeatureFrameInfo& featureInfo
) {
    m_commandBuffer = commandBuffer;
    m_renderTarget = &renderTarget;
    m_splatResources = &splatResources;
    m_splatFrameDescriptorSet = splatFrameDescriptorSet;
    m_splatGaussianPipeline = splatGaussianPipeline;
    m_pipelineLayout = pipelineLayout;
    m_featureInfo = &featureInfo;
}

bool FrameCommands::record( 
    const FramePushConstant& pushConstants
) {
    Logger* logger = Logger::get_logger();

    if(!is_valid())
    {
        logger->print("Invalid frame command buffer");
        return false;
    }

    if(m_commandBuffer.reset() != vk::Result::eSuccess)
    {
        logger->print("Failed to reset frame command buffer");
        return false;
    }

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    if(m_commandBuffer.begin(beginInfo) != vk::Result::eSuccess)
    {
        logger->print("Failed to begin frame command buffer");
        return false;
    }

    transition_image_layout(
        m_commandBuffer,
        m_renderTarget->colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eGeneral,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eShaderWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::ImageAspectFlagBits::eColor
    );

    m_commandBuffer.pushConstants(
        m_pipelineLayout,
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eVertex 
        ),
        0,
        sizeof(FramePushConstant),
        &pushConstants
    );

    record_background_pass(
        m_commandBuffer, 
        m_featureInfo->pipelines.background, 
        m_pipelineLayout, 
        m_renderTarget->storageDescriptorSet, 
        m_renderTarget->extent
    );

    record_splat_preprocess_pass(
        m_commandBuffer,
        m_featureInfo->pipelines.splatPreprocess,
        m_pipelineLayout,
        m_featureInfo->sphericalHarmonicDescriptorSet,
        m_splatFrameDescriptorSet,
        *m_splatResources
    );

    record_splat_entry_scan_pass(
        m_commandBuffer, 
        m_featureInfo->pipelines.splatEntryScan, 
        m_pipelineLayout,
        m_splatFrameDescriptorSet,
        *m_splatResources
    );

    record_splat_tile_pass(
        m_commandBuffer,
        m_featureInfo->pipelines.splatTile,
        m_pipelineLayout,
        m_splatFrameDescriptorSet,
        *m_splatResources
    );

// FLAG //
    uint32_t sortedBufferIndex = record_splat_sort_pass(
        m_commandBuffer,
        m_featureInfo->pipelines.splatSort,
        m_pipelineLayout,
        m_splatFrameDescriptorSet,
        *m_splatResources
    );
/////

    record_splat_tile_range_pass(
        m_commandBuffer,
        m_featureInfo->pipelines.splatTileRange,
        m_pipelineLayout,
        m_splatFrameDescriptorSet,
        *m_splatResources,
        sortedBufferIndex
    );

    transition_image_layout(
        m_commandBuffer,
        m_renderTarget->colorImage,
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eGeneral,
        vk::AccessFlagBits::eShaderWrite,
        (
            vk::AccessFlagBits::eShaderRead | 
            vk::AccessFlagBits::eShaderWrite
        ),
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::ImageAspectFlagBits::eColor
    );

    record_splat_tile_render_pass(
        m_commandBuffer,
        m_featureInfo->pipelines.splatTileRender,
        m_pipelineLayout,
        m_renderTarget->storageDescriptorSet,
        m_splatFrameDescriptorSet,
        m_renderTarget->extent,
        sortedBufferIndex
    );

    vk::BufferMemoryBarrier counterCopyBarrier = make_buffer_memory_barrier(
        m_splatResources->counters.buffer,
        m_splatResources->counters.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eTransferRead
    );

    m_commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        nullptr,
        counterCopyBarrier,
        nullptr
    );

    vk::BufferCopy counterCopyRegion = {};

    counterCopyRegion.srcOffset = 0;
    counterCopyRegion.dstOffset = 0;
    counterCopyRegion.size = sizeof(GpuSplatCounters);

    m_commandBuffer.copyBuffer(
        m_splatResources->counters.buffer,
        m_splatResources->counterReadback.buffer,
        1,
        &counterCopyRegion
    );

    vk::BufferMemoryBarrier counterReadbackBarrier = make_buffer_memory_barrier(
        m_splatResources->counterReadback.buffer,
        m_splatResources->counterReadback.size,
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eHostRead
    );

    m_commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eHost,
        vk::DependencyFlags(),
        nullptr,
        counterReadbackBarrier,
        nullptr
    );

    transition_image_layout(
        m_commandBuffer,
        m_renderTarget->colorImage,
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eNone,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );    

    if(m_commandBuffer.end() != vk::Result::eSuccess)
    {
        logger->print("Failed to end frame command buffer");
        return false;
    }

    return true;
}

bool FrameCommands::is_valid() const
{
    return (
        m_commandBuffer &&
        m_renderTarget &&
        m_splatResources &&
        m_splatFrameDescriptorSet &&
        m_splatGaussianPipeline &&
        m_pipelineLayout &&
        m_featureInfo &&
        splat_frame_resources_are_valid(*m_splatResources) &&
        render_target_is_valid(*m_renderTarget) &&
        render_feature_frame_info_is_valid(*m_featureInfo)
    );
}

void FrameCommands::build_rendering_info(
    vk::RenderingInfoKHR& renderingInfo,
    vk::RenderingAttachmentInfoKHR& colorAttachment,
    vk::RenderingAttachmentInfoKHR& depthAttachment
) const {
    renderingInfo.flags = vk::RenderingFlagsKHR();
    renderingInfo.renderArea = vk::Rect2D({0, 0}, m_renderTarget->extent);
    renderingInfo.viewMask = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = nullptr;
}

void FrameCommands::build_color_attachment(
    vk::RenderingAttachmentInfoKHR& colorAttachment
) const {
    colorAttachment.imageView = m_renderTarget->colorImageView;
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
}

void FrameCommands::build_depth_attachment(
    vk::RenderingAttachmentInfoKHR& depthAttachment
) const {
    vk::ClearValue clearValue = {};
    clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    depthAttachment.imageView = m_renderTarget->depthImage.imageView;
    depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.clearValue = clearValue;
}

void FrameCommands::initialize_render_state() 
{
    vk::Viewport viewport = vk::Viewport(
        0.0f, 
        0.0f, 
        static_cast<float>(m_renderTarget->extent.width), 
        static_cast<float>(m_renderTarget->extent.height), 
        0.0f, 
        1.0f
    );

    vk::Rect2D scissor = vk::Rect2D({0, 0}, m_renderTarget->extent);

    m_commandBuffer.setViewport(0, 1, &viewport);
    m_commandBuffer.setScissor(0, 1, &scissor);
}

bool render_target_is_valid(
    const RenderTarget& target
) {
    return (
        target.colorImage &&
        target.colorImageView &&
        target.storageDescriptorSet &&
        target.depthImage.image &&
        target.depthImage.imageView 
    );
}
