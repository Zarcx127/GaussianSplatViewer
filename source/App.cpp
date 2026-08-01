#include "App.hpp"

#include <sstream>

namespace
{
    constexpr double EVENT_WAIT_TIMEOUT = 1.0 / 120.0;
}

App::App(GLFWwindow* window, Engine* engine)
{
    m_window = window;
    m_engine = engine;

    m_logger = Logger::get_logger();

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    state.framebufferWidth = width;
    state.framebufferHeight = height;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, window_resize_callback);
    glfwSetKeyCallback(window, App::key_callback);
    glfwSetMouseButtonCallback(window, App::mouse_button_callback);
    glfwSetCursorPosCallback(window, App::cursor_pos_callback);

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    std::lock_guard<std::mutex> lock(m_inputMutex);
    
    m_inputState.mouseX = mouseX;
    m_inputState.mouseY = mouseY;
}

void App::main_loop()
{
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

        uint32_t fps = state.fps.load(std::memory_order_relaxed);

        std::stringstream title;
        title << "Running at " << fps << " fps";

        glfwSetWindowTitle(m_window, title.str().c_str());
    }

    state.quitRequested.store(true, std::memory_order_release);

    if(m_renderThread.joinable())
        m_renderThread.join();

    m_logger->print("Window Closed");
}

InputState App::snapshot_input() 
{
    std::lock_guard<std::mutex> lock(m_inputMutex);

    InputState snapshot = m_inputState;

    m_inputState.mouseDeltaX = 0.0;
    m_inputState.mouseDeltaY = 0.0;

    return snapshot;
}

App::~App()
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

void App::window_resize_callback(GLFWwindow* window, int width, int height)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    app->state.framebufferWidth.store(width, std::memory_order_relaxed);
    app->state.framebufferHeight.store(height, std::memory_order_relaxed);

    app->state.resizeGeneration.fetch_add(1, std::memory_order_release);
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if(!app) return;

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

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if(!app) return;

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

void App::cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
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
