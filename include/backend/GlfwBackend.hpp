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

#ifndef BACKEND_GLFW_BACKEND_H
#define BACKEND_GLFW_BACKEND_H

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

class GlfwBackend
{
public:
    GlfwBackend(int width, int height, const char* name);

    GlfwBackend(const GlfwBackend&) = delete;
    GlfwBackend& operator=(const GlfwBackend&) = delete;

    GlfwBackend(GlfwBackend&&) = delete;
    GlfwBackend& operator=(GlfwBackend&&) = delete;

    bool build_window();
    GLFWwindow* get_window();

    ~GlfwBackend();

private:
    GLFWwindow* m_window { nullptr };

    int m_width;
    int m_height;

    const char* m_name;
};

#endif
