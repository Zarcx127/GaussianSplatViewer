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
