#pragma once

#ifndef APP_STATE_H
#define APP_STATE_H

#include <atomic>
#include <cstdint>

enum class RenderStatus : uint32_t
{
    NotStarted,
    Initializing,
    Running,
    InitFailed,
    FatalError,
    Stopping,
    Stopped
};

struct AppState
{
    std::atomic<bool> quitRequested { false };
    
    std::atomic<int> framebufferWidth { 0 };
    std::atomic<int> framebufferHeight { 0 };

    std::atomic<uint64_t> resizeGeneration { 0 };
    
    std::atomic<uint32_t> fps { 0 };

    std::atomic<RenderStatus> renderStatus { RenderStatus::NotStarted };
};

#endif
