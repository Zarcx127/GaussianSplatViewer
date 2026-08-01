#pragma once

#ifndef APP_H
#define APP_H

#include <glfw/glfw3.h>

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"

class App
{
public:
    App(GLFWwindow* window, Engine* engine);
    ~App();

    void main_loop();

private:
    Logger* m_logger { nullptr };

    GLFWwindow* m_window { nullptr };
    Engine* m_engine { nullptr };
};

#endif
