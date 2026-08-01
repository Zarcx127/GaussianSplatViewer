#include "renderer/core/Image.hpp"

#include "logging/Logger.hpp"

vk::ImageView create_image_view(vk::Device logicalDevice, vk::Image image, vk::Format format)
{
    Logger* logger = Logger::get_logger();

    vk::ImageViewCreateInfo createInfo = {};
    
    createInfo.image = image;
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = format;

    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;
    
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    vk::ResultValue<vk::ImageView> attemptImageView = logicalDevice.createImageView(createInfo);
    if(attemptImageView.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create image view");
        return vk::ImageView();
    }

    return attemptImageView.value;
}

void transition_image_layout(
    vk::CommandBuffer commandBuffer, 
    vk::Image image, 
    vk::ImageLayout oldLayout, 
    vk::ImageLayout newLayout, 
    vk::AccessFlags srcAccessMask, 
    vk::AccessFlags dstAccessMask,
    vk::PipelineStageFlags srcStage, 
    vk::PipelineStageFlags dstStage
) {
    vk::ImageSubresourceRange access = {};
    
    access.aspectMask = vk::ImageAspectFlagBits::eColor;
    access.baseMipLevel = 0;
    access.levelCount = 1;
    access.baseArrayLayer = 0;
    access.layerCount = 1;

    vk::ImageMemoryBarrier barrier = {};

    barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = access;

    barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;

    commandBuffer.pipelineBarrier(
        srcStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier
    );
} 
