#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"
#include "backend/GlfwBackend.hpp"

#include "App.hpp"

int main()
{
    Logger* logger = Logger::get_logger();
    logger->set_mode(true);

    int width = 800, height = 600;
    GLFWwindow* window = build_window(width, height, "ID Tech 12");

    Engine* engine = new Engine(window);
    App* app = new App(window, engine);

    delete app;
    delete engine;

    glfwDestroyWindow(window);
    glfwTerminate();

    Logger::clean_up();

    return 0;
}

