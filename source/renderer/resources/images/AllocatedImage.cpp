#include "renderer/resources/images/AllocatedImage.hpp"

#include <vector>

#include "logging/Logger.hpp"

#include "renderer/core/Image.hpp"

vk::Format find_depth_format(vk::PhysicalDevice physicalDevice)
{
    std::vector<vk::Format> candidates = {
        vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16Unorm
    };

    for(vk::Format format : candidates)
    {
        vk::FormatProperties properties = physicalDevice.getFormatProperties(format);
        if(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            return format;
    }

    return vk::Format::eUndefined;
}

AllocatedImage create_depth_image(
    VmaAllocator allocator,
    vk::Device logicalDevice,
    vk::PhysicalDevice physicalDevice,
    vk::Extent2D extent,
    std::deque<std::function<void(vk::Device)>>& renderDeletionQueue,
    std::deque<std::function<void(VmaAllocator)>>& renderVmaDeletionQueue
) {
    Logger* logger = Logger::get_logger();

    AllocatedImage depthImage = {};
    depthImage.format = find_depth_format(physicalDevice);
    depthImage.extent = extent;

    if(depthImage.format == vk::Format::eUndefined)
    {
        logger->print("Failed to find depth format");
        return {};
    }

    vk::ImageCreateInfo imageInfo = {};
    
    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = static_cast<vk::Format>(depthImage.format);
    imageInfo.extent = vk::Extent3D {
        extent.width,
        extent.height,
        1
    };

    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo allocationInfo = {};
    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkImage rawImage = {};
    VmaAllocation rawAllocation = VK_NULL_HANDLE;

    vk::Result vmaImageAttempt = static_cast<vk::Result>(
        vmaCreateImage(
            allocator, 
            &static_cast<VkImageCreateInfo&>(imageInfo),
            &allocationInfo,
            &rawImage,
            &rawAllocation, 
            nullptr
        )
    );

    if(vmaImageAttempt != vk::Result::eSuccess)
    {
        logger->print("Failed to create depth image");
        return {};
    }

    depthImage.image = vk::Image(rawImage);
    depthImage.allocation = rawAllocation;

    depthImage.imageView = create_image_view(
        logicalDevice,
        depthImage.image,
        depthImage.format,
        vk::ImageAspectFlagBits::eDepth
    );

    if(!depthImage.imageView)
    {
        logger->print("Failed to create depth image view");
        vmaDestroyImage(allocator, rawImage, rawAllocation);

        return {};
    }

    VkImage imageHandle = rawImage;
    VmaAllocation allocationHandle = rawAllocation;
    renderVmaDeletionQueue.push_back(
        [logger, imageHandle, allocationHandle] (VmaAllocator allocator)->void {
            vmaDestroyImage(allocator, imageHandle, allocationHandle);
            logger->print("Deleted depth image");
        }
    );

    vk::ImageView imageViewHandle = depthImage.imageView;
    renderDeletionQueue.push_back(
        [logger, imageViewHandle] (vk::Device device)->void {
            device.destroyImageView(imageViewHandle);
            logger->print("Deleted depth image view");
        }
    );

    logger->print("Created depth image");

    return depthImage;
}
