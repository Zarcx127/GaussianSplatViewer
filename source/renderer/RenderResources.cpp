#include "renderer/RenderResources.hpp"

#include "factory/SplatFactory.hpp"
#include "factory/SplatFrameFactory.hpp"

#include "renderer/core/Command.hpp"
#include "renderer/core/Image.hpp"

#include "renderer/resources/descriptors/Descriptors.hpp"

#include "renderer/resources/shaders/Shader.hpp"

#include "renderer/resources/splats/SplatFrameDescriptors.hpp"

#include "renderer/ShaderPaths.hpp"

bool RenderResources::build(
    RenderResourcesContext& context,
    uint32_t width,
    uint32_t height,
    uint32_t framesInFlight,
    const SplatBuffer& splatBuffer,
    uint32_t entryCapacity
) {
    uint32_t splatCapacity = splatBuffer.splatCount;

    if(
        !splatBuffer.buffer.buffer ||
        (width == 0) || 
        (height == 0) || 
        (framesInFlight == 0) ||
        (entryCapacity == 0) ||
        (splatCapacity == 0) ||
        !render_resources_context_is_valid(context)
    ) {
        return false;
    }

    bool swapchainAttemptState = m_swapchain.build(
        context.logicalDevice, context.physicalDevice, 
        context.surface, width, height, m_deletionQueue
    );

    if(!swapchainAttemptState || m_swapchain.imageViews.empty())
    {
        destroy(context);
        return false;
    }

    uint32_t swapchainDescriptorCount = 
        static_cast<uint32_t>(m_swapchain.imageViews.size());

    uint32_t splatFrameDescriptorCount = framesInFlight;
    uint32_t descriptorSetCount = (
        swapchainDescriptorCount + splatFrameDescriptorCount
    );

    DescriptorPoolBuilder descriptorPoolBuilder(context.logicalDevice);

    descriptorPoolBuilder.add_entry(
        vk::DescriptorType::eStorageImage,
        swapchainDescriptorCount
    );

    descriptorPoolBuilder.add_entry(
        vk::DescriptorType::eStorageBuffer,
        (splatFrameDescriptorCount * SPLAT_FRAME_BINDING_COUNT)
    );

    m_descriptorPool = descriptorPoolBuilder.build(
        descriptorSetCount,
        m_deletionQueue
    );

    m_swapchainImageDescriptorSets.clear();
    m_swapchainImageDescriptorSets.resize(m_swapchain.imageViews.size());

    for(uint32_t i = 0; i < m_swapchain.imageViews.size(); i++)
    {
        m_swapchainImageDescriptorSets[i] = allocate_descriptor_set(
            context.logicalDevice, m_descriptorPool,
            context.renderInterface.get_descriptor_set_layouts()[0]
        );

        if(!m_swapchainImageDescriptorSets[i])
        {
            destroy(context);
            return false;
        }

        write_storage_image_descriptor(
            context.logicalDevice, 
            m_swapchainImageDescriptorSets[i],
            m_swapchain.imageViews[i], 
            vk::ImageLayout::eGeneral
        );
    }

    m_depthImage = create_depth_image(
        context.allocator, context.logicalDevice, 
        context.physicalDevice, m_swapchain.extent, 
        m_deletionQueue, m_vmaDeletionQueue
    );

    if(!m_depthImage.image || !m_depthImage.imageView)
    {
        destroy(context);
        return false;
    }

    GraphicsPipelineConfig splatPipelineConfig = get_splat_gaussian_pipeline_config();
    m_splatGaussianPipeline = make_graphics_pipeline(
        context.logicalDevice,
        shader::splatGaussianVertex,
        shader::splatGaussianFragment,
        splatPipelineConfig,
        context.pipelineLayout,
        m_swapchain.format.format,
        m_depthImage.format,
        m_deletionQueue
    );

    if(!m_splatGaussianPipeline)
    {
        destroy(context);
        return false;
    }

    bool depthTransitionSuccessful = immediate_submit(
        context.logicalDevice, context.commandPool, context.graphicsQueue,
        [this] (vk::CommandBuffer commandBuffer)->void {
            transition_image_layout(
                commandBuffer,
                m_depthImage.image,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthAttachmentOptimal,
                vk::AccessFlagBits::eNone,
                vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::ImageAspectFlagBits::eDepth
            );
        }
    );

    if(!depthTransitionSuccessful)
    {
        destroy(context);
        return false;
    }

    uint32_t tileCountX = (
        (m_swapchain.extent.width + SPLAT_TILE_SIZE - 1) /
        SPLAT_TILE_SIZE
    );

    uint32_t tileCountY = (
        (m_swapchain.extent.height + SPLAT_TILE_SIZE - 1) /
        SPLAT_TILE_SIZE
    );

    uint32_t tileCapacity = (tileCountX * tileCountY);
    if(tileCapacity == 0)
    {
        destroy(context);
        return false;
    }

    m_frames.reserve(framesInFlight);
    for(uint32_t i = 0; i < framesInFlight; i++)
    {
        vk::CommandBuffer commandBuffer = make_command_buffer(
            context.logicalDevice, context.commandPool, m_deletionQueue
        );

        if(!commandBuffer)
        {
            destroy(context);
            return false;
        }

        SplatFrameResources splatResources = build_splat_frame_resources(
            context.allocator,
            splatCapacity,
            entryCapacity,
            tileCapacity,
            m_vmaDeletionQueue
        );

        if(!splat_frame_resources_are_valid(splatResources))
        {
            destroy(context);
            return false;
        }

        vk::DescriptorSet splatFrameDescriptorSet = allocate_descriptor_set(
            context.logicalDevice,
            m_descriptorPool,
            context.splatFrameDescriptorSetLayout
        );

        if(!splatFrameDescriptorSet)
        {
            destroy(context);
            return false;
        }

        if(!write_splat_frame_descriptor_set(
            context.logicalDevice,
            splatFrameDescriptorSet,
            splatBuffer,
            splatResources
        )) {
            destroy(context);
            return false;
        }

        m_frames.emplace_back(
            context.logicalDevice, commandBuffer, 
            splatResources, splatFrameDescriptorSet,
            static_cast<uint32_t>(m_swapchain.imageCount),
            m_deletionQueue
        );

        if(!m_frames.back().is_valid(static_cast<uint32_t>(m_swapchain.imageCount)))
        {
            destroy(context);
            return false;
        }
    }

    m_imagesInFlight.resize(m_swapchain.imageViews.size(), vk::Fence());

    return true;
}

void RenderResources::destroy(RenderResourcesContext& context)
{
    if(context.graphicsQueue)
        (void) context.graphicsQueue.waitIdle();

    m_frames.clear();
    m_imagesInFlight.clear();
    m_swapchainImageDescriptorSets.clear();

    while(!m_deletionQueue.empty())
    {
        if(context.logicalDevice)
            (m_deletionQueue.back())(context.logicalDevice);
        
        m_deletionQueue.pop_back();
    }

    while(!m_vmaDeletionQueue.empty())
    {
        if(context.allocator)
            (m_vmaDeletionQueue.back())(context.allocator);
        
        m_vmaDeletionQueue.pop_back();
    }

    m_descriptorPool = vk::DescriptorPool();
    m_splatGaussianPipeline = vk::Pipeline();
    m_swapchain = {};
    m_depthImage = {};
}

vk::SwapchainKHR RenderResources::swapchain_handle() const
{
    return m_swapchain.chain;
}

vk::Extent2D RenderResources::extent() const
{
    return m_swapchain.extent;
}

vk::Image RenderResources::color_image(uint32_t imageIndex) const
{
    return m_swapchain.images[imageIndex];
}

vk::ImageView RenderResources::color_image_view(uint32_t imageIndex) const
{
    return m_swapchain.imageViews[imageIndex];
}

uint32_t RenderResources::color_image_count() const
{
    return static_cast<uint32_t>(m_swapchain.images.size());
}

uint32_t RenderResources::color_image_view_count() const
{
    return static_cast<uint32_t>(m_swapchain.imageViews.size());
}

vk::Pipeline RenderResources::splat_gaussian_pipeline() const
{
    return m_splatGaussianPipeline;
}

const AllocatedImage& RenderResources::depth_image() const
{
    return m_depthImage;
}

vk::DescriptorSet RenderResources::swapchain_storage_descriptor_set(uint32_t imageIndex) const
{
    return m_swapchainImageDescriptorSets[imageIndex];
}

uint32_t RenderResources::swapchain_storage_descriptor_set_count() const
{
    return static_cast<uint32_t>(m_swapchainImageDescriptorSets.size());
}

Frame& RenderResources::frame(uint32_t index)
{
    return m_frames[index];
}

uint32_t RenderResources::frame_count() const
{
    return static_cast<uint32_t>(m_frames.size());
}

vk::Fence& RenderResources::image_in_flight(uint32_t imageIndex)
{
    return m_imagesInFlight[imageIndex];
}

uint32_t RenderResources::image_in_flight_count() const
{
    return static_cast<uint32_t>(m_imagesInFlight.size());
}

bool render_resources_context_is_valid(
    const RenderResourcesContext& context
) {
    return (
        context.logicalDevice &&
        context.physicalDevice &&
        context.surface &&
        context.allocator &&
        context.commandPool &&
        context.graphicsQueue &&
        context.pipelineLayout &&
        !context.renderInterface.get_descriptor_set_layouts().empty()
    );
}