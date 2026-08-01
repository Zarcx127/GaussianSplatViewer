#pragma once

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

struct SurfaceDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class Swapchain
{
public:
    uint32_t imageCount;
    vk::SwapchainKHR chain;

    vk::SurfaceFormatKHR format {};
    vk::Extent2D extent {};

    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;

    bool build(
        vk::Device logicalDevice, 
        vk::PhysicalDevice physicalDevice, 
        vk::SurfaceKHR surface, 
        uint32_t width, 
        uint32_t height, 
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

private:
    SurfaceDetails query_surface_support(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

    vk::Extent2D choose_extent(uint32_t width, uint32_t height, vk::SurfaceCapabilitiesKHR capabilities);

    vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& presentModes);

    vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
};

#endif
