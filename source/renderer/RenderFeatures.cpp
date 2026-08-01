#include "renderer/RenderFeatures.hpp"

#include "assets/splats/PlySplatLoader.hpp"

#include "factory/SplatFactory.hpp"

#include "logging/Logger.hpp"

#include "renderer/resources/shaders/Shader.hpp"

bool RenderFeatures::build(RenderFeaturesContext& context, const char* splatPath)
{
    Logger* logger = Logger::get_logger();

    if(
        !context.logicalDevice ||
        !context.allocator ||
        !context.commandPool ||
        !context.graphicsQueue ||
        !context.pipelineLayout
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
    m_backgroundPipeline = vk::Pipeline();
}

RenderFeatureFrameInfo RenderFeatures::frame_info() const
{
    return {
        m_splatBuffer,
        m_backgroundPipeline
    };
}