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

#ifndef VIEWER_H
#define VIEWER_H

#include <mutex>
#include <thread>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include "backend/GlfwBackend.hpp"

#include "input/InputState.hpp"

#include "renderer/Renderer.hpp"

#include "ViewerState.hpp"
#include "ApplicationSession.hpp"

class Viewer
{
public:
    ViewerState state;
    
    Viewer(GlfwBackend& backend, Engine& engine);

    ViewerResult main_loop();

    InputState snapshot_input();

    ~Viewer();

private:
    GlfwBackend* m_backend { nullptr };
    GLFWwindow* m_window { nullptr };

    Engine* m_engine { nullptr };

    std::thread m_renderThread;

    std::mutex m_inputMutex;
    InputState m_inputState {};

    static void window_resize_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xPos, double yPos);
};

#endif
