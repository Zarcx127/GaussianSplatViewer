#include "renderer/RenderFeatures.hpp"

#include "factory/MeshFactory.hpp"

#include "renderer/resources/shaders/Shader.hpp"

bool RenderFeatures::build(RenderFeaturesContext& context)
{
    if(
        !context.logicalDevice ||
        !context.allocator ||
        !context.commandPool ||
        !context.graphicsQueue ||
        !context.pipelineLayout
    ) {
        return false;
    }

    m_debugMesh = build_cube(
        context.allocator, context.logicalDevice, context.commandPool,
        context.graphicsQueue, m_vmaDeletionQueue
    );

    if(!m_debugMesh.vertexBuffer.buffer)
    {
        destroy(context);
        return false;
    }

    m_worldGridPipeline = make_compute_pipeline(
        context.logicalDevice,  "shaders/bin/WorldGrid.comp.spv", 
        context.pipelineLayout, m_deletionQueue
    );

    if(!m_worldGridPipeline)
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

    m_debugMesh = {};
    m_worldGridPipeline = vk::Pipeline();
}

RenderFeatureFrameInfo RenderFeatures::frame_info() const
{
    return {
        m_debugMesh,
        m_worldGridPipeline
    };
}