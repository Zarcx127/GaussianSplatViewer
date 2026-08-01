#include "App.hpp"

#ifdef APP_H

using std::atomic;
using std::thread;

static void window_resize_callback(GLFWwindow* window, int width, int height);

App::App(GLFWwindow* window, Engine* engine)
{
    m_window = window;
    m_engine = engine;

    m_logger = Logger::get_logger();

    glfwSetWindowUserPointer(window, engine);
    glfwSetFramebufferSizeCallback(window, window_resize_callback);
    
    main_loop();
} 

void App::main_loop()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        m_engine->draw();
        m_engine->update_timing();
    }

    m_running = false;
    m_logger->print("Window Closed");
}

App::~App()
{
    glfwSetWindowUserPointer(m_window, nullptr);
    glfwSetFramebufferSizeCallback(m_window, nullptr);
}

static void window_resize_callback(GLFWwindow* window, int width, int height)
{
    Engine* engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->rebuildSwapchain = true;
}

#endif
