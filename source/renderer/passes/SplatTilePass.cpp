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

#include "renderer/passes/SplatTilePass.hpp"

#include <array>

#include "renderer/core/Synchronization.hpp"

namespace
{
    constexpr uint32_t SPLAT_TILE_LOCAL_SIZE = 256;
}

void record_splat_tile_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
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

    uint32_t groupCount = (
        (resources.splatCapacity + SPLAT_TILE_LOCAL_SIZE - 1) /
        SPLAT_TILE_LOCAL_SIZE
    );

    commandBuffer.dispatch(groupCount, 1, 1);

    std::array<vk::BufferMemoryBarrier, 2> outputBarriers = {
        make_buffer_memory_barrier(
            resources.entryKeys[0].buffer,
            resources.entryKeys[0].size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        ),
        make_buffer_memory_barrier(
            resources.entrySplatIndices[0].buffer,
            resources.entrySplatIndices[0].size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        )
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        outputBarriers,
        nullptr
    );
}