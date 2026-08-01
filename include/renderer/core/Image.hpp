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
