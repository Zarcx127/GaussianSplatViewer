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

#ifndef BACKEND_WIN_BACKEND_H
#define BACKEND_WIN_BACKEND_H

#include <functional>

#include <windows.h>

class WinBackend
{
public:
    WinBackend(int width, int height, const char* name);

    WinBackend(const WinBackend&) = delete;
    WinBackend& operator=(const WinBackend&) = delete;

    WinBackend(WinBackend&&) = delete;
    WinBackend& operator=(WinBackend&&) = delete;

    std::function<void(UINT)> dpiChangedCallback;
    std::function<bool(HDC)> eraseBackgroundCallback;
    std::function<HBRUSH(HDC, HWND)> controlColorCallback;
    std::function<bool(const DRAWITEMSTRUCT*)> drawItemCallback;
    std::function<bool(int, int)> commandCallback;
    std::function<void()> closeCallback;

    bool build_window();

    HWND get_window();
    UINT get_dpi();

    void clear_callbacks();

    bool set_window_colors(
        COLORREF captionColor,
        COLORREF textColor,
        COLORREF borderColor
    );

    void show_window();
    void close_window();

    bool wait_events();

    ~WinBackend();

private:
    HINSTANCE m_instance { nullptr };
    HWND m_window { nullptr };

    int m_width;
    int m_height;

    const char* m_name { nullptr };

    UINT m_dpi { USER_DEFAULT_SCREEN_DPI };

    bool m_classRegistered { false };

    static LRESULT CALLBACK window_procedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );
};

#endif
