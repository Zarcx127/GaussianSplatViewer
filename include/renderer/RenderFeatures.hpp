#pragma once

#ifndef RENDERER_RENDER_FEATURES_H
#define RENDERER_RENDER_FEATURES_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

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
    vk::Pipeline backgroundPipeline;
    vk::Pipeline splatCullPipeline;
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

    vk::Pipeline m_backgroundPipeline {};
    vk::Pipeline m_splatCullPipeline {};
};

#endif
