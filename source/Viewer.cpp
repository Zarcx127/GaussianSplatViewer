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

#include "Viewer.hpp"

#include <sstream>

#include "logging/Logger.hpp"

namespace
{
    constexpr double EVENT_WAIT_TIMEOUT = 1.0 / 120.0;
}

Viewer::Viewer(GlfwBackend& backend, Engine& engine)
{
    m_backend = &backend;
    m_engine = &engine;
    
    m_window = m_backend->get_window();

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    state.framebufferWidth = width;
    state.framebufferHeight = height;

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, window_resize_callback);
    glfwSetKeyCallback(m_window, Viewer::key_callback);
    glfwSetMouseButtonCallback(m_window, Viewer::mouse_button_callback);
    glfwSetCursorPosCallback(m_window, Viewer::cursor_pos_callback);

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    std::lock_guard<std::mutex> lock(m_inputMutex);
    
    m_inputState.mouseX = mouseX;
    m_inputState.mouseY = mouseY;
}

ViewerResult Viewer::main_loop()
{
    Logger* logger = Logger::get_logger();
    
    bool resizeEnabled = false;

    m_renderThread = std::thread(
        [this] ()->void {
            m_engine->render_loop(
                state,
                [this] ()->InputState {
                    return snapshot_input();
                }
            );
        }
    );

    while(
        !glfwWindowShouldClose(m_window) &&
        !state.quitRequested.load(std::memory_order_acquire)
    ) {
        glfwWaitEventsTimeout(EVENT_WAIT_TIMEOUT);

        RenderStatus renderStatus = state.renderStatus.load(std::memory_order_acquire);

        if(
            !resizeEnabled &&
            (renderStatus == RenderStatus::Running)
        ) {
            glfwSetWindowAttrib(
                m_window,
                GLFW_RESIZABLE,
                GLFW_TRUE
            );

            resizeEnabled = true;
        }

        uint32_t fps = state.fps.load(std::memory_order_relaxed);

        std::stringstream title;
        title << "Running at " << fps << " fps";

        glfwSetWindowTitle(m_window, title.str().c_str());
    }

    state.quitRequested.store(true, std::memory_order_release);

    if(m_renderThread.joinable())
        m_renderThread.join();

    logger->print("Window Closed");

    const RenderStatus renderStatus = 
        state.renderStatus.load(std::memory_order_acquire);

    if(renderStatus == RenderStatus::InitFailed)
    {
        if(m_engine->load_error())
            return ViewerResult::LoadFailed;

        return ViewerResult::FatalError;
    }

    if(renderStatus == RenderStatus::FatalError)
        return ViewerResult::FatalError;

    return ViewerResult::ReturnToLauncher;
}

InputState Viewer::snapshot_input() 
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    InputState snapshot = m_inputState;

    m_inputState.mouseDeltaX = 0.0;
    m_inputState.mouseDeltaY = 0.0;

    return snapshot;
}

void Viewer::window_resize_callback(GLFWwindow* window, int width, int height)
{
    Viewer* app = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    app->state.framebufferWidth.store(width, std::memory_order_relaxed);
    app->state.framebufferHeight.store(height, std::memory_order_relaxed);

    app->state.resizeGeneration.fetch_add(1, std::memory_order_release);
}

void Viewer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Viewer* app = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    RenderStatus renderStatus = 
        app->state.renderStatus.load(std::memory_order_acquire);

    if(renderStatus != RenderStatus::Running)
        return;

    bool pressed = (action != GLFW_RELEASE);

    std::lock_guard<std::mutex> lock(app->m_inputMutex);

    switch(key)
    {
        case GLFW_KEY_W: app->m_inputState.keyW = pressed; break;
        case GLFW_KEY_A: app->m_inputState.keyA = pressed; break;
        case GLFW_KEY_S: app->m_inputState.keyS = pressed; break;
        case GLFW_KEY_D: app->m_inputState.keyD = pressed; break;
        case GLFW_KEY_E: app->m_inputState.keyE = pressed; break;
        case GLFW_KEY_Q: app->m_inputState.keyQ = pressed; break;
    }
}

void Viewer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Viewer* app = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    RenderStatus renderStatus = 
        app->state.renderStatus.load(std::memory_order_acquire);

    if(renderStatus != RenderStatus::Running)
        return;

    if(button != GLFW_MOUSE_BUTTON_RIGHT)
        return;

    std::lock_guard<std::mutex> lock(app->m_inputMutex);

    if(action == GLFW_PRESS)
    {
        double mouseX = 0.0;
        double mouseY = 0.0;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        app->m_inputState.rightMouseDown = true;
        app->m_inputState.mouseX = mouseX;
        app->m_inputState.mouseY = mouseY;
        app->m_inputState.mouseDeltaX = 0.0;
        app->m_inputState.mouseDeltaY = 0.0;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if(action == GLFW_RELEASE)
    {
        app->m_inputState.rightMouseDown = false;
        app->m_inputState.mouseDeltaX = 0.0;
        app->m_inputState.mouseDeltaY = 0.0;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Viewer::cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
{
    Viewer* app = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    std::lock_guard<std::mutex> lock(app->m_inputMutex);

    double dx = xPos - app->m_inputState.mouseX;
    double dy = yPos - app->m_inputState.mouseY;

    app->m_inputState.mouseX = xPos;
    app->m_inputState.mouseY = yPos;

    if(app->m_inputState.rightMouseDown)
    {
        app->m_inputState.mouseDeltaX += dx;
        app->m_inputState.mouseDeltaY += dy;
    }
}

Viewer::~Viewer()
{
    state.quitRequested.store(true, std::memory_order_release);

    if(m_renderThread.joinable())
        m_renderThread.join();

    glfwSetWindowUserPointer(m_window, nullptr);
    glfwSetFramebufferSizeCallback(m_window, nullptr);
    glfwSetKeyCallback(m_window, nullptr);
    glfwSetMouseButtonCallback(m_window, nullptr);
    glfwSetCursorPosCallback(m_window, nullptr);
}
