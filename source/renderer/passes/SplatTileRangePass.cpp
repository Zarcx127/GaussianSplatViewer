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

#include "renderer/passes/SplatTileRangePass.hpp"

#include "backend/Utils.hpp"

#include "renderer/core/Synchronization.hpp"

#include "renderer/FrameCommands.hpp"

namespace
{
    constexpr uint32_t SPLAT_TILE_RANGE_LOCAL_SIZE = 256;
    constexpr uint32_t SPLAT_TILE_RANGE_DISPATCH_ROW_CAPACITY = 65535;
};

void record_splat_tile_range_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources,
    uint32_t sortedBufferIndex
) {
    commandBuffer.fillBuffer(
        resources.tileRanges.buffer,
        0,
        resources.tileRanges.size,
        0
    );

    vk::BufferMemoryBarrier clearBarrier = make_buffer_memory_barrier(
        resources.tileRanges.buffer,
        resources.tileRanges.size,
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderWrite
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        clearBarrier,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipeline
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

    SplatTileRangePushConstant pushConstants = {};
    pushConstants.sortedBufferIndex = sortedBufferIndex;

    commandBuffer.pushConstants(
        pipelineLayout,
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eFragment
        ),
        sizeof(FramePushConstant),
        sizeof(SplatTileRangePushConstant),
        &pushConstants
    );

    uint32_t workgroupCapacity = utils::divide_round_up(
        resources.entryCapacity,
        SPLAT_TILE_RANGE_LOCAL_SIZE
    );

    uint32_t groupCountY = utils::divide_round_up(
        workgroupCapacity,
        SPLAT_TILE_RANGE_DISPATCH_ROW_CAPACITY
    );

    uint32_t groupCountX = utils::divide_round_up(
        workgroupCapacity,
        groupCountY
    );

    commandBuffer.dispatch(
        groupCountX,
        groupCountY,
        1
    );

    vk::BufferMemoryBarrier rangeBarrier = make_buffer_memory_barrier(
        resources.tileRanges.buffer,
        resources.tileRanges.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eShaderRead
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        rangeBarrier,
        nullptr
    );
}
