#pragma once

#ifndef APPLICATION_SESSION_H
#define APPLICATION_SESSION_H

#include <cstdint>

enum class ViewerResult : uint32_t 
{
    ReturnToLauncher,
    LoadFailed,
    FatalError
};

enum class LauncherAction : uint32_t
{
    OpenViewer,
    ExitApplication
};

struct LauncherResult
{
    LauncherAction action { LauncherAction::ExitApplication };
    const char* filePath { nullptr };
};

#endif
