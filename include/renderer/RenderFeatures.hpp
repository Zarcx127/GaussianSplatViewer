#pragma once

#ifndef RENDERER_RENDER_FEATURES_H
#define RENDERER_RENDER_FEATURES_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "renderer/resources/pipeline/RenderFeaturePipelines.hpp"

#include "renderer/resources/splats/Splat.hpp"

struct RenderFeaturesContext
{
    vk::Device logicalDevice;

    VmaAllocator allocator { nullptr };

    vk::CommandPool commandPool;
    vk::Queue graphicsQueue;

    vk::PipelineLayout pipelineLayout;

    vk::DescriptorSetLayout sphericalHarmonicDescriptorSetLayout;
};

struct RenderFeatureFrameInfo
{
    const SplatBuffer& splatBuffer;
    const SphericalHarmonicBuffer& sphericalHarmonicBuffer;
    
    vk::DescriptorSet sphericalHarmonicDescriptorSet;
    
    const RenderFeaturePipelines& pipelines;
};

class RenderFeatures
{
public:
    RenderFeatures() = default;

    RenderFeatures(const RenderFeatures&) = delete;
    RenderFeatures& operator=(const RenderFeatures&) = delete;

    RenderFeatures(RenderFeatures&&) = delete;
    RenderFeatures& operator=(RenderFeatures&&) = delete;

    bool build(RenderFeaturesContext& context, const char* splatPath);
    void destroy(RenderFeaturesContext& context);

    RenderFeatureFrameInfo frame_info() const;

private:
    std::deque<std::function<void(vk::Device)>> m_deletionQueue;
    std::deque<std::function<void(VmaAllocator)>> m_vmaDeletionQueue;

    SplatBuffer m_splatBuffer;

    SphericalHarmonicBuffer m_sphericalHarmonicBuffer;

    vk::DescriptorPool m_descriptorPool {};
    vk::DescriptorSet m_sphericalHarmonicDescriptorSet {};

    RenderFeaturePipelines m_pipelines {};
};

bool render_features_context_is_valid(
    const RenderFeaturesContext& context
);

bool render_feature_frame_info_is_valid(
    const RenderFeatureFrameInfo& frameInfo
);

#endif
