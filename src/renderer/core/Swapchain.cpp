#include "renderer/core/Swapchain.hpp"

#include <algorithm>

#include "logging/Logger.hpp"

#include "renderer/core/Image.hpp"

bool Swapchain::build(
    vk::Device logicalDevice, 
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    uint32_t width, 
    uint32_t height, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    chain = vk::SwapchainKHR();
    
    images.clear();
    imageViews.clear();

    imageCount = 0;
    format = vk::SurfaceFormatKHR();
    extent = vk::Extent2D();

    SurfaceDetails support = query_surface_support(physicalDevice, surface);

    if(support.formats.empty())
    {
        logger->print("Swapchain has no supported surface formats");
        return false;
    }

    if(support.presentModes.empty())
    {
        logger->print("Swapchain has no supported present modes");
        return false;
    }

    format = choose_surface_format(support.formats);
    if(format.format == vk::Format::eUndefined)
    {
        logger->print("Swapchain does not support required surface format");
        return false;
    }

    vk::PresentModeKHR presentMode = choose_present_mode(support.presentModes);
    if(presentMode == vk::PresentModeKHR())
    {
        logger->print("Swapchain failed to choose present mode");
        return false;
    }

    extent = choose_extent(width, height, support.capabilities);
    if((extent.width == 0) || (extent.height == 0)) 
    {
        logger->print("Swapchain failed to create because drawable extent is invalid");
        return false;
    }

    imageCount = support.capabilities.minImageCount + 1;
    if(support.capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, support.capabilities.maxImageCount);

    vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    if(support.capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eStorage)
    {
        imageUsage |= vk::ImageUsageFlagBits::eStorage;
    }
    else
    {
        logger->print("Swapchain images do not support storage image usage");
        return false;
    }

    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        imageCount,
        format.format,
        format.colorSpace,
        extent,
        1,
        imageUsage,
        vk::SharingMode::eExclusive
    );

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.presentMode = presentMode;
    createInfo.clipped = vk::True;
    createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

    vk::SwapchainKHR newSwapchain = vk::SwapchainKHR();
    vk::Result swapChainAttempt = logicalDevice.createSwapchainKHR(
        &createInfo, nullptr, &newSwapchain
    );
    
    if(swapChainAttempt != vk::Result::eSuccess)
    {
        logger->print("Failed to create swapchain");
        return false;
    }

    chain = newSwapchain;

    vk::SwapchainKHR chainHandle = chain;
    deletionQueue.push_back(
        [logger, chainHandle] (vk::Device device)->void {
            device.destroySwapchainKHR(chainHandle);
            logger->print("Deleted swapchain");
        }
    );
    
    vk::ResultValue<std::vector<vk::Image>> imagesAttempt = 
        logicalDevice.getSwapchainImagesKHR(chain);

    if(imagesAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to get images");
        return false;
    }

    images = imagesAttempt.value;
    imageCount = static_cast<uint32_t>(images.size());

    imageViews.resize(images.size());
    for(uint32_t i = 0; i < images.size(); i++)
    {
        imageViews[i] = create_image_view(logicalDevice, images[i], format.format);

        if(!imageViews[i])
        {
            logger->print("Failed to create swapchain image view");
            return false;
        }

        vk::ImageView imageViewHandle = imageViews[i];
        deletionQueue.push_back(
            [logger, imageViewHandle] (vk::Device device)->void {
                device.destroyImageView(imageViewHandle);
                logger->print("Deleted image view");
            }
        );
    }

    return true;
}

SurfaceDetails Swapchain::query_surface_support(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface
) {
    Logger* logger = Logger::get_logger();

    SurfaceDetails support;
    support.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
    logger->log(support.capabilities);

    support.formats = physicalDevice.getSurfaceFormatsKHR(surface).value;
    logger->log(support.formats);

    support.presentModes = physicalDevice.getSurfacePresentModesKHR(surface).value;
    logger->log(support.presentModes);

    return support;
}

vk::Extent2D Swapchain::choose_extent(
    uint32_t width, 
    uint32_t height, 
    vk::SurfaceCapabilitiesKHR capabilities
) {
    if(capabilities.currentExtent.width != UINT32_MAX) 
        return capabilities.currentExtent;

    vk::Extent2D extent;

    extent.width = std::min(
        capabilities.maxImageExtent.width,
        std::max(capabilities.minImageExtent.width, width)
    );

    extent.height = std::min(
        capabilities.maxImageExtent.height,
        std::max(capabilities.minImageExtent.height, height)
    );

    return extent;
}

vk::PresentModeKHR Swapchain::choose_present_mode(const std::vector<vk::PresentModeKHR>& presentModes)
{
    Logger* logger = Logger::get_logger();
    for(const vk::PresentModeKHR& mode : presentModes)
    {
        if(mode == vk::PresentModeKHR::eMailbox)
        {
            logger->print("Swapchain using mailbox present mode");
            return mode;
        }
    }

    for(const vk::PresentModeKHR& mode : presentModes)
    {
        if(mode == vk::PresentModeKHR::eFifo)
        {
            logger->print("Swapchain using fifo present mode");
            return mode;
        }
    }

    return vk::PresentModeKHR();
}

vk::SurfaceFormatKHR Swapchain::choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for(const vk::SurfaceFormatKHR& format : formats)
    {
        if(
            (format.format == vk::Format::eR8G8B8A8Unorm) && 
            (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        ) {
            return format;
        }
    }

    return vk::SurfaceFormatKHR();
}
