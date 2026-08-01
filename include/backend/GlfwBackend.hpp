#pragma once

#ifndef GLFW_BACKEND_H
#define GLFW_BACKEND_H

#include <glfw/glfw3.h>

#include "logging/Logger.hpp"

class Window
{
public:
    Window(int width, int height, const char* name);

    bool build_window();
    GLFWwindow* get_window();

    ~Window();

private:
    Logger* m_logger { nullptr };

    GLFWwindow* m_window { nullptr };

    int m_width;
    int m_height;

    const char* m_name;
};

#endif
