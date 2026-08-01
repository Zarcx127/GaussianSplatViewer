#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"
#include "backend/GlfwBackend.hpp"

#include "App.hpp"
#include "AppState.hpp"

int main()
{
    Logger* logger = Logger::get_logger();
    logger->set_mode(true);

    int width = 800, height = 600;
    Window window(width, height, "Graphics");
    if(!window.build_window())
        return 1;

    Engine engine(window.get_window());

    App app(window.get_window(), &engine);
    app.main_loop();

    if(
        (app.state.renderStatus == RenderStatus::InitFailed) ||
        (app.state.renderStatus == RenderStatus::FatalError)
    ) {
        logger->print("ERROR: Engine crashed!!!");
        return 1;
    }

    return 0;
}
