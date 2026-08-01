#include "backend/GlfwBackend.hpp"

#ifdef GLFW_BACKEND_H

#include <sstream>

#include "logging/Logger.hpp"

GLFWwindow* build_window(int width, int height, const char* name)
{
    Logger* logger = Logger::get_logger();

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    if(window)
    {
        std::stringstream line;

        line << "Successfully made a glfw window called \"" << name
            << "\", width = " << width
            << ", height = " << height;

        logger->print(line.str().c_str());
    }
    else
    {
        logger->print("GLFW window creation failed");
    }

    return window;
}

#endif
