#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <string>

#include <windows.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"
#include "backend/GlfwBackend.hpp"

#include "Viewer.hpp"
#include "AppState.hpp"
#include "Launcher.hpp"

bool run_launcher(std::string* selectedFilePath, LauncherResult* result);
ViewerResult run_viewer(const char* splatPath, std::string* loadError);

int main()
{
    Logger* logger = Logger::get_logger();
    logger->set_mode(true);

    std::string selectedFilePath;
    while(true)
    {
        LauncherResult launcherResult {};

        if(!run_launcher(&selectedFilePath, &launcherResult))
        {
            logger->print("ERROR: Launcher failed to initialize");
            return 1;
        }

        if(launcherResult.action == LauncherAction::ExitApplication)
            break;

        std::string loadError;
        ViewerResult viewerResult = run_viewer(
            launcherResult.filePath,
            &loadError
        );

        if(viewerResult == ViewerResult::LoadFailed)
        {
            MessageBoxA(
                nullptr,
                loadError.c_str(),
                "Failed to Open File",
                MB_OK | MB_ICONERROR
            );

            continue;
        }

        if(viewerResult == ViewerResult::FatalError)
        {
            logger->print("ERROR: Engine failed");
            return 1;
        }
    }

    return 0;
}

bool run_launcher(std::string* selectedFilePath, LauncherResult* result)
{
    Launcher launcher(selectedFilePath);

    if(!launcher.build())
        return false;

    *result = launcher.main_loop();

    return true;
}

ViewerResult run_viewer(const char* splatPath, std::string* loadError)
{
    int width = 800;
    int height = 600;

    Window window(width, height, "Graphics");

    if(!window.build_window())
        return ViewerResult::FatalError;

    Engine engine(window.get_window(), splatPath);
    Viewer viewer(window.get_window(), &engine);

    ViewerResult result = viewer.main_loop();

    if((result == ViewerResult::LoadFailed) && engine.load_error())
        *loadError = engine.load_error();

    return result;
}
