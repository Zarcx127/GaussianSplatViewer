#include "App.hpp"

#include <sstream>

static constexpr double EVENT_WAIT_TIMEOUT = 1.0 / 120.0;

static void window_resize_callback(GLFWwindow* window, int width, int height);

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
}

void App::main_loop()
{
    m_renderThread = std::thread(
        [this] ()->void {
            m_engine->render_loop(state);
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

App::~App()
{
    state.quitRequested.store(true, std::memory_order_release);

    if(m_renderThread.joinable())
        m_renderThread.join();

    glfwSetWindowUserPointer(m_window, nullptr);
    glfwSetFramebufferSizeCallback(m_window, nullptr);
}

static void window_resize_callback(GLFWwindow* window, int width, int height)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if(!app) return;

    app->state.framebufferWidth.store(width, std::memory_order_relaxed);
    app->state.framebufferHeight.store(height, std::memory_order_relaxed);

    app->state.resizeGeneration.fetch_add(1, std::memory_order_release);
}
