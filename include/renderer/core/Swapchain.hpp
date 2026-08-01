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

#ifndef RENDERER_CORE_SWAPCHAIN_H
#define RENDERER_CORE_SWAPCHAIN_H

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
    uint32_t imageCount { 0 };
    vk::SwapchainKHR chain {};

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
