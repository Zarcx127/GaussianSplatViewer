#include "renderer/RenderFeatures.hpp"

#include "assets/splats/PlySplatLoader.hpp"

#include "factory/SplatFactory.hpp"

#include "logging/Logger.hpp"

#include "renderer/resources/descriptors/Descriptors.hpp"

#include "renderer/resources/shaders/Shader.hpp"

bool RenderFeatures::build(RenderFeaturesContext& context, const char* splatPath)
{
    Logger* logger = Logger::get_logger();

    if(
        !context.logicalDevice ||
        !context.allocator ||
        !context.commandPool ||
        !context.graphicsQueue ||
        !context.pipelineLayout ||
        !context.sphericalHarmonicDescriptorSetLayout
    ) {
        return false;
    }

    PlySplatLoadResult loadResult = load_ply_splat_cloud(splatPath);
    if(!loadResult.success)
    {
        logger->print(loadResult.error);
        return false;
    }

    m_splatBuffer = build_splat_buffer(
        loadResult.cloud,
        context.allocator, context.logicalDevice, context.commandPool,
        context.graphicsQueue, m_vmaDeletionQueue
    );

    if(!m_splatBuffer.buffer.buffer || (m_splatBuffer.splatCount == 0))
    {
        destroy(context);
        return false;
    }

    m_sphericalHarmonicBuffer = build_spherical_harmonic_buffer(
        loadResult.cloud,
        context.allocator, context.logicalDevice, context.commandPool,
        context.graphicsQueue, m_vmaDeletionQueue
    );

    if(!m_sphericalHarmonicBuffer.buffer.buffer)
    {
        destroy(context);
        return false;
    }

    DescriptorPoolBuilder descriptorPoolBuilder(context.logicalDevice);
    descriptorPoolBuilder.add_entry(vk::DescriptorType::eStorageBuffer, 1);

    m_descriptorPool = descriptorPoolBuilder.build(1, m_deletionQueue);
    if(!m_descriptorPool)
    {
        destroy(context);
        return false;
    }

    m_sphericalHarmonicDescriptorSet = allocate_descriptor_set(
        context.logicalDevice,
        m_descriptorPool,
        context.sphericalHarmonicDescriptorSetLayout
    );

    if(!m_sphericalHarmonicDescriptorSet)
    {
        destroy(context);
        return false;
    }

    write_storage_buffer_descriptor(
        context.logicalDevice,
        m_sphericalHarmonicDescriptorSet,
        m_sphericalHarmonicBuffer.buffer.buffer,
        0,
        m_sphericalHarmonicBuffer.buffer.size
    );

    m_backgroundPipeline = make_compute_pipeline(
        context.logicalDevice,  "shaders/bin/Background.comp.spv", 
        context.pipelineLayout, m_deletionQueue
    );

    if(!m_backgroundPipeline)
    {
        destroy(context);
        return false;
    }

    return true;
}

void RenderFeatures::destroy(RenderFeaturesContext& context)
{
    if(context.graphicsQueue)
        (void) context.graphicsQueue.waitIdle();

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

    m_splatBuffer = {};
    m_sphericalHarmonicBuffer = {};

    m_descriptorPool = vk::DescriptorPool();
    m_sphericalHarmonicDescriptorSet = vk::DescriptorSet();

    m_backgroundPipeline = vk::Pipeline();
}

RenderFeatureFrameInfo RenderFeatures::frame_info() const
{
    return {
        m_splatBuffer,
        m_sphericalHarmonicBuffer,
        m_sphericalHarmonicDescriptorSet,
        m_backgroundPipeline
    };
}
