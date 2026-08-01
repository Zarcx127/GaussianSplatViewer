#pragma once

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <string>

#include <windows.h>

#include "ApplicationSession.hpp"

class Launcher
{
public:
    Launcher(std::string* selectedFilePath);

    Launcher(const Launcher&) = delete;
    Launcher& operator=(const Launcher&) = delete;

    Launcher(Launcher&&) = delete;
    Launcher& operator=(Launcher&&) = delete;

    bool build();

    LauncherResult main_loop();

    ~Launcher();

private:
    HINSTANCE m_instance { nullptr };

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

    std::string* m_selectedFilePath { nullptr };

    LauncherResult m_result {};

    bool build_controls();
    bool select_file();

    void layout_controls();
    void layout_file_path_text();

    void draw_button(const DRAWITEMSTRUCT* drawInfo);
    
    static LRESULT CALLBACK window_proc(
        HWND window, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
    );
};

#endif
