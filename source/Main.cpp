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

#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <string>
#include <filesystem>

#include <windows.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"
#include "backend/GlfwBackend.hpp"

#include "Viewer.hpp"
#include "ViewerState.hpp"
#include "Launcher.hpp"

LauncherResult run_launcher(std::filesystem::path& selectedFilePath);
ViewerResult run_viewer(
    const std::filesystem::path& splatPath, 
    std::string& loadError
);

int main()
{
    Logger* logger = Logger::get_logger();
    logger->set_mode(true);

    std::filesystem::path selectedFilePath;
    while(true)
    {
        LauncherResult launcherResult = run_launcher(selectedFilePath);
        if(launcherResult == LauncherResult::FatalError)
        {
            logger->print("ERROR: Launcher failed to initialize");
            return 1;
        }

        if(launcherResult == LauncherResult::ExitApplication)
            break;

        std::string loadError;
        ViewerResult viewerResult = run_viewer(selectedFilePath, loadError);
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

LauncherResult run_launcher(std::filesystem::path& selectedFilePath)
{
    int width = 440;
    int height = 134;

    WinBackend backend(width, height, "Gaussian Splat Launcher");
    if(!backend.build_window())
        return LauncherResult::FatalError;

    Launcher launcher(backend, selectedFilePath);
    if(!launcher.build())
        return LauncherResult::FatalError;

    return launcher.main_loop();
}

ViewerResult run_viewer(
    const std::filesystem::path& splatPath, 
    std::string& loadError
) {
    int width = 800;
    int height = 600;

    GlfwBackend backend(width, height, "Graphics");
    if(!backend.build_window())
        return ViewerResult::FatalError;

    Engine engine(backend.get_window(), splatPath);
    Viewer viewer(backend, engine);

    ViewerResult result = viewer.main_loop();
    if((result == ViewerResult::LoadFailed) && engine.load_error())
        loadError = engine.load_error();

    return result;
}
