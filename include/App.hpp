#pragma once

#ifndef APP_H
#define APP_H

#include <mutex>
#include <thread>

#include <glfw/glfw3.h>

#include "AppState.hpp"

#include "logging/Logger.hpp"

#include "input/InputState.hpp"

#include "renderer/Renderer.hpp"

class App
{
public:
    AppState state;

    App(GLFWwindow* window, Engine* engine);
    ~App();

    void main_loop();

    InputState snapshot_input();

private:
    Logger* m_logger { nullptr };

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
