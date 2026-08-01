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

#ifndef RENDERER_FRAME_COMMANDS_H
#define RENDERER_FRAME_COMMANDS_H

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

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
    vk::Extent2D extent;

    vk::DescriptorSet storageDescriptorSet;
};

class FrameCommands
{
public:
    FrameCommands(
        vk::CommandBuffer commandBuffer,
        const RenderTarget& renderTarget,
        const SplatFrameResources& splatResources,
        vk::DescriptorSet splatFrameDescriptorSet,
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
    vk::PipelineLayout m_pipelineLayout {};

    const RenderFeatureFrameInfo* m_featureInfo { nullptr };

    bool is_valid() const;
};

bool render_target_is_valid(const RenderTarget& target);

#endif
