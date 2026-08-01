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

#ifndef RENDERER_CORE_IMAGE_H
#define RENDERER_CORE_IMAGE_H

#include <vulkan/vulkan.hpp>

vk::ImageView create_image_view(
    vk::Device device, 
    vk::Image image, 
    vk::Format format,
    vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor
);

void transition_image_layout(
    vk::CommandBuffer commandBuffer, 
    vk::Image image, 
    vk::ImageLayout oldLayout, 
    vk::ImageLayout newLayout, 
    vk::AccessFlags srcAccessMask, 
    vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStage, 
    vk::PipelineStageFlags dstStage,
    vk::ImageAspectFlags aspectMask
); 

#endif
