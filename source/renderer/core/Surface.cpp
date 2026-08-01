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

#include "renderer/core/Surface.hpp"

#include "logging/Logger.hpp"

vk::SurfaceKHR make_vulkan_surface(
    const vk::Instance& instance, 
    GLFWwindow* window, 
    std::deque<std::function<void(vk::Instance)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    if(!window)
    {
        logger->print("Cannot create Vulkan surface without a GLFW window");
        return vk::SurfaceKHR();
    }

    if(!instance)
    {
        logger->print("Cannot create Vulkan surface without a Vulkan instance");
        return vk::SurfaceKHR();
    }

    VkSurfaceKHR rawSurface = VK_NULL_HANDLE;
    VkResult rawSurfaceResult = glfwCreateWindowSurface(instance, window, nullptr, &rawSurface);
    if(rawSurfaceResult != VK_SUCCESS)
    {
        logger->print("Failed to create window surface");
        return vk::SurfaceKHR();
    }

    vk::SurfaceKHR surface = vk::SurfaceKHR(rawSurface);
    deletionQueue.push_back(
        [logger, surface] (vk::Instance instance)->void {
            instance.destroySurfaceKHR(surface);
            logger->print("Deleted Vulkan surface");
        }
    );

    return surface;
}
