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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <deque>
#include <functional>
#include <filesystem>

#include "ApplicationSession.hpp"
#include "backend/WinBackend.hpp"

class Launcher
{
public:
    Launcher(WinBackend& backend,  std::filesystem::path& selectedFilePath);

    Launcher(const Launcher&) = delete;
    Launcher& operator=(const Launcher&) = delete;

    Launcher(Launcher&&) = delete;
    Launcher& operator=(Launcher&&) = delete;

    bool build();

    LauncherResult main_loop();

    ~Launcher();

private:
    std::deque<std::function<void()>> m_brushDeletionQueue;

    WinBackend* m_backend { nullptr };
    HWND m_window { nullptr };

    HWND m_filePathLabel { nullptr };
    HWND m_filePathControl { nullptr };
    HWND m_selectFileButton { nullptr };
    HWND m_openFileButton { nullptr };

    HBRUSH m_backgroundBrush { nullptr };
    HBRUSH m_controlBrush { nullptr };
    HBRUSH m_buttonBrush { nullptr };
    HBRUSH m_buttonPressedBrush { nullptr };
    HBRUSH m_buttonDisabledBrush { nullptr };
    HBRUSH m_borderBrush { nullptr };

    UINT m_dpi { USER_DEFAULT_SCREEN_DPI };

    bool m_comInitialized { false };

    std::filesystem::path* m_selectedFilePath { nullptr };

    LauncherResult m_result { LauncherResult::Running };

    bool build_controls();
    bool select_file();

    void layout_controls();
    void layout_file_path_text();

    void draw_button(const DRAWITEMSTRUCT* drawInf);

    void dpi_changed_callback(UINT dpi);
    bool erase_background_callback(HDC deviceContext);
    HBRUSH control_color_callback(HDC deviceContext, HWND control);
    bool draw_item_callback(const DRAWITEMSTRUCT* drawInfo);
    bool command_callback(int controlId, int notification);

    bool create_brush(HBRUSH& brush, COLORREF color);

    int scale_dimension(int value, UINT dpi);
};

#endif
