#pragma once

#ifndef RENDERER_CORE_SURFACE_H
#define RENDERER_CORE_SURFACE_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

vk::SurfaceKHR make_vulkan_surface(
    const vk::Instance& instance, 
    GLFWwindow* window,
    std::deque<std::function<void(vk::Instance)>>& deletionQueue
);

#endif
