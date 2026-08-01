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
