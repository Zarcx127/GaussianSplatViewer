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

#include "renderer/passes/BackgroundPass.hpp"

void record_background_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet descriptorSet,
    vk::Extent2D extent
) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    uint32_t pixelCount = extent.width * extent.height;
    uint32_t groupCount = (pixelCount + 63) / 64;

    commandBuffer.dispatch(groupCount, 1, 1);
}