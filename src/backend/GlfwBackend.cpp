#include "backend/GlfwBackend.hpp"

#include <sstream>

Window::Window(int width, int height, const char* name)
{
    m_logger = Logger::get_logger();
    
    m_width = width;
    m_height = height;
    m_name = name;
}

bool Window::build_window()
{
    if(!glfwInit())
    {
        m_logger->print("GLFW Initialization failed");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_name, nullptr, nullptr);
    if(!m_window)
    {
        m_logger->print("GLFW window creation failed");
        return false;
    }

    std::stringstream line;

    line << "Successfully made a glfw window called \"" << m_name
        << "\", width = " << m_width
        << ", height = " << m_height;

    m_logger->print(line.str().c_str());

    return true;
}

GLFWwindow* Window::get_window()
{
    return m_window; 
}

Window::~Window()
{
    if(m_window)
        glfwDestroyWindow(m_window);
    
    glfwTerminate();
}
