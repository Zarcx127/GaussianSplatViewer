#pragma once

#ifndef APP_H
#define APP_H

#include <thread>

#include <glfw/glfw3.h>

#include "AppState.hpp"

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"

class App
{
public:
    AppState state;

    App(GLFWwindow* window, Engine* engine);
    ~App();

    void main_loop();

private:
    Logger* m_logger { nullptr };

    GLFWwindow* m_window { nullptr };
    Engine* m_engine { nullptr };

    std::thread m_renderThread;
};

#endif
