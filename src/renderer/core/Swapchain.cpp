#include "renderer/core/Swapchain.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Image.hpp"

void Swapchain::build(
    vk::Device logicalDevice, 
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    uint32_t width, 
    uint32_t height, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    SurfaceDetails support = query_surface_support(physicalDevice, surface);

    format = choose_surface_format(support.formats);
    vk::PresentModeKHR presentMode = choose_present_mode(support.presentModes);

    extent = choose_extent(width, height, support.capabilities);
    imageCount = support.capabilities.minImageCount + 1;
    if(support.capabilities.maxImageCount > 0)
        imageCount = std::min(imageCount, support.capabilities.maxImageCount);

    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        imageCount,
        format.format,
        format.colorSpace,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive
    );

    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

    vk::ResultValue<vk::SwapchainKHR> swapChainAttempt = logicalDevice.createSwapchainKHR(createInfo);
    if(swapChainAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create swapchain");
        return;
    }

    chain = swapChainAttempt.value;
    
    vk::ResultValue<std::vector<vk::Image>> imagesAttempt = 
        logicalDevice.getSwapchainImagesKHR(chain);

    if(imagesAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to get images");
        return;
    }

    images = imagesAttempt.value;

    imageViews.resize(images.size());
    for(uint32_t i = 0; i < images.size(); i++)
    {
        imageViews[i] = create_image_view(logicalDevice, images[i], format.format);

        vk::ImageView imageViewHandle = imageViews[i];
        deletionQueue.push_back([imageViewHandle, logger] (vk::Device device)->void{
            device.destroyImageView(imageViewHandle);
            logger->print("Deleted image view");
        });
    }

    vk::SwapchainKHR chainHandle = chain;
    deletionQueue.push_back([chainHandle, logger] (vk::Device device)->void{
        device.destroySwapchainKHR(chainHandle);
        logger->print("Deleted swapchain");
    });
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
    for(vk::PresentModeKHR mode : presentModes)
        if(mode == vk::PresentModeKHR::eMailbox)
            return mode;

    return vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR Swapchain::choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for(vk::SurfaceFormatKHR format : formats)
    {
        if(
            (format.format == vk::Format::eB8G8R8A8Unorm) 
            && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        ) {
            return format;
        }
    }

    return formats[0];
}
