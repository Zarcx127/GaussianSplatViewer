/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#ifndef RENDERER_RENDER_FEATURES_H
#define RENDERER_RENDER_FEATURES_H

#include <deque>
#include <functional>
#include <filesystem>

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

    bool build(
        RenderFeaturesContext& context, 
        const std::filesystem::path& splatPath,
        const char*& loadError,
        const std::function<bool()>& progressCallback
    );
    
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
