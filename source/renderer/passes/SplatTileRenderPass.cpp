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

#include "renderer/passes/SplatTileRenderPass.hpp"

#include "renderer/resources/splats/SplatFrame.hpp"

#include "renderer/FrameCommands.hpp"

void record_splat_tile_render_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet renderTargetDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    vk::Extent2D extent,
    uint32_t sortedBufferIndex
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipeline
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        0,
        1,
        &renderTargetDescriptorSet,
        0,
        nullptr
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        2,
        1,
        &splatFrameDescriptorSet,
        0,
        nullptr
    );

    SplatTileRenderPushConstant pushConstants = {};
    pushConstants.sortedBufferIndex = sortedBufferIndex;

    commandBuffer.pushConstants(
        pipelineLayout,
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eFragment
        ),
        sizeof(FramePushConstant),
        sizeof(SplatTileRenderPushConstant),
        &pushConstants
    );

    uint32_t groupCountX = (
        (extent.width / SPLAT_TILE_SIZE) +
        static_cast<uint32_t>(
            (extent.width % SPLAT_TILE_SIZE) != 0
        )
    );

    uint32_t groupCountY = (
        (extent.height / SPLAT_TILE_SIZE) +
        static_cast<uint32_t>(
            (extent.height % SPLAT_TILE_SIZE) != 0
        )
    );

    commandBuffer.dispatch(
        groupCountX,
        groupCountY,
        1
    );
}