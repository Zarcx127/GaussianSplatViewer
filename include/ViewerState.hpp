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

#pragma once

#ifndef VIEWER_STATE_H
#define VIEWER_STATE_H

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

struct ViewerState
{
    std::atomic<bool> quitRequested { false };
    
    std::atomic<int> framebufferWidth { 0 };
    std::atomic<int> framebufferHeight { 0 };

    std::atomic<uint64_t> resizeGeneration { 0 };
    
    std::atomic<uint32_t> fps { 0 };

    std::atomic<RenderStatus> renderStatus { RenderStatus::NotStarted };
};

#endif
