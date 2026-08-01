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

#include "backend/GlfwBackend.hpp"

#include <sstream>

#include "logging/Logger.hpp"

GlfwBackend::GlfwBackend(int width, int height, const char* name)
{
    m_width = width;
    m_height = height;
    m_name = name;
}

bool GlfwBackend::build_window()
{
    Logger* logger = Logger::get_logger();

    if(!glfwInit())
    {
        logger->print("GLFW Initialization failed");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_name, nullptr, nullptr);
    if(!m_window)
    {
        logger->print("GLFW window creation failed");
        return false;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if(monitor)
    {
        int monitorX = 0;
        int monitorY = 0;
        int monitorWidth = 0;
        int monitorHeight = 0;

        glfwGetMonitorWorkarea(
            monitor,
            &monitorX,
            &monitorY,
            &monitorWidth,
            &monitorHeight
        );

        int windowWidth = 0;
        int windowHeight = 0;

        glfwGetWindowSize(
            m_window,
            &windowWidth,
            &windowHeight
        );

        glfwSetWindowPos(
            m_window,
            (monitorX + (monitorWidth - windowWidth) / 2),
            (monitorY + (monitorHeight - windowHeight) / 2)
        );
    }

    std::stringstream line;
    line << "Successfully made a glfw window called \"" << m_name
        << "\", width = " << m_width
        << ", height = " << m_height;

    logger->print(line.str().c_str());

    return true;
}

GLFWwindow* GlfwBackend::get_window()
{
    return m_window; 
}

GlfwBackend::~GlfwBackend()
{
    if(m_window)
        glfwDestroyWindow(m_window);
    
    glfwTerminate();
}
