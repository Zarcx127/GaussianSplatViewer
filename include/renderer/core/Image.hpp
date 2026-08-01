#pragma once

#ifndef IMAGE_H
#define IMAGE_H

#include <vulkan/vulkan.hpp>

vk::ImageView create_image_view(vk::Device logicalDevice, vk::Image image, vk::Format format);

void transition_image_layout(
    vk::CommandBuffer commandBuffer, 
    vk::Image image, 
    vk::ImageLayout oldLayout, 
    vk::ImageLayout newLayout, 
    vk::AccessFlags srcAccessMask, 
    vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStage, 
    vk::PipelineStageFlags dstStage
); 

#endif
