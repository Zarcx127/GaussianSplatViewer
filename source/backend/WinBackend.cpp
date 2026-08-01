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

#include "backend/WinBackend.hpp"

#include <dwmapi.h>

#include "logging/Logger.hpp"

namespace
{
    constexpr const char* WINDOW_CLASS_NAME = "GaussianSplatLauncherWindow";

    constexpr DWORD WINDOW_STYLE = (
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX
    );
}

WinBackend::WinBackend(int width, int height, const char* name)
{
    m_width = width;
    m_height = height;
    m_name = name;
}

bool WinBackend::build_window()
{
    Logger* logger = Logger::get_logger();

    m_instance = GetModuleHandleA(nullptr);
    if(!m_instance)
    {
        logger->print("Failed to get Win32 application instance");
        return false;
    }

    WNDCLASSEXA windowClass {};

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = (CS_HREDRAW | CS_VREDRAW);
    windowClass.lpfnWndProc = WinBackend::window_procedure;
    windowClass.hInstance = m_instance;
    windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if(!RegisterClassExA(&windowClass))
    {
        logger->print("Failed to register launcher Window class");
        return false;
    }

    m_classRegistered = true;

    logger->print("Registered launcher window class");

    m_window = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        m_name,
        WINDOW_STYLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_width,
        m_height,
        nullptr,
        nullptr,
        m_instance,
        this
    );

    if(!m_window)
    {
        logger->print("Failed to create launcher window");
        return false;
    }

    logger->print("Created launcher window");

    m_dpi = GetDpiForWindow(m_window);

    RECT windowRect = {
        0,
        0,
        MulDiv(m_width, m_dpi, USER_DEFAULT_SCREEN_DPI),
        MulDiv(m_height, m_dpi, USER_DEFAULT_SCREEN_DPI)
    };

    if(!AdjustWindowRectExForDpi(
        &windowRect,
        WINDOW_STYLE,
        FALSE,
        0,
        m_dpi
    )) {
        logger->print("Failed to calculate launcher window dimensions");
        return false;
    }

    const int windowWidth = (windowRect.right - windowRect.left);
    const int windowHeight = (windowRect.bottom - windowRect.top);

    HMONITOR monitor = MonitorFromWindow(
        m_window,
        MONITOR_DEFAULTTOPRIMARY
    );

    MONITORINFO monitorInfo {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if(!GetMonitorInfoA(monitor, &monitorInfo))
    {
        logger->print("Failed to get launcher monitor information");
        return false;
    }

    const int workWidth = (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
    const int workHeight = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
    
    const int windowX = (
        monitorInfo.rcWork.left +
        ((workWidth - windowWidth) / 2)
    );

    const int windowY = (
        monitorInfo.rcWork.top +
        ((workHeight - windowHeight) / 2)
    );
    
    SetWindowPos(
        m_window,
        nullptr,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    logger->print("Configured launcher window");

    return true;
}

HWND WinBackend::get_window()
{
    return m_window;
}

UINT WinBackend::get_dpi()
{
    return m_dpi;
}

void WinBackend::clear_callbacks()
{
    eraseBackgroundCallback = {};
    dpiChangedCallback = {};
    controlColorCallback = {};
    drawItemCallback = {};
    commandCallback = {};
    closeCallback = {};
}

bool WinBackend::set_window_colors(
    COLORREF captionColor,
    COLORREF textColor,
    COLORREF borderColor
) {
    if(!m_window)
        return false;

    (void) DwmSetWindowAttribute(
        m_window,
        DWMWA_CAPTION_COLOR,
        &captionColor,
        sizeof(captionColor)
    );

    (void) DwmSetWindowAttribute(
        m_window,
        DWMWA_TEXT_COLOR,
        &textColor,
        sizeof(textColor)
    );

    (void) DwmSetWindowAttribute(
        m_window,
        DWMWA_BORDER_COLOR,
        &borderColor,
        sizeof(borderColor)
    );

    return true;
}

void WinBackend::show_window()
{
    if(!m_window)
        return;

    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);
}

void WinBackend::close_window()
{
    if(
        m_window &&
        IsWindow(m_window)
    ) {
        DestroyWindow(m_window);
    }
}

bool WinBackend::wait_events()
{
    Logger* logger = Logger::get_logger();

    MSG message {};

    const BOOL result = GetMessageA(
        &message,
        nullptr,
        0,
        0
    );

    if(result == -1)
    {
        logger->print("Failed to read Win32 window event");
        return false;
    }

    if(result == 0)
        return false;

    TranslateMessage(&message);
    DispatchMessageA(&message);

    return true;
}

LRESULT CALLBACK WinBackend::window_procedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    Logger* logger = Logger::get_logger();

    WinBackend* backend;
    if(message == WM_NCCREATE)
    {
        CREATESTRUCTA* createInfo = reinterpret_cast<CREATESTRUCTA*>(lParam);
        backend = reinterpret_cast<WinBackend*>(createInfo->lpCreateParams);

        SetWindowLongPtrA(
            window,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(backend)
        );

        backend->m_window = window;
    }

    backend = reinterpret_cast<WinBackend*>(
        GetWindowLongPtrA(window, GWLP_USERDATA)
    );

    if(!backend)
        return DefWindowProcA(window, message, wParam, lParam);

    if(message == WM_DPICHANGED) 
    {
        backend->m_dpi = HIWORD(wParam);

        RECT* suggestedRect = reinterpret_cast<RECT*>(lParam);

        SetWindowPos(
            window,
            nullptr,
            suggestedRect->left,
            suggestedRect->top,
            (suggestedRect->right - suggestedRect->left),
            (suggestedRect->bottom - suggestedRect->top),
            (SWP_NOZORDER | SWP_NOACTIVATE)
        );

        if(backend->dpiChangedCallback)
            backend->dpiChangedCallback(backend->m_dpi);

        return 0;
    }

    if(
        backend->eraseBackgroundCallback &&
        (message == WM_ERASEBKGND)
    ) {
        if(backend->eraseBackgroundCallback(reinterpret_cast<HDC>(wParam)))
            return 1;
    }

    if(message == WM_CTLCOLORSTATIC)
    {
        HBRUSH brush = nullptr;

        if(backend->controlColorCallback)
        {
            brush = backend->controlColorCallback(
                reinterpret_cast<HDC>(
                    wParam
                ),
                reinterpret_cast<HWND>(
                    lParam
                )
            );
        }

        if(brush)
            return reinterpret_cast<LRESULT>(brush);
    }

    if(
        backend->drawItemCallback &&
        (message == WM_DRAWITEM) 
    ) {
        const DRAWITEMSTRUCT* drawInfo = 
            reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);

        if(backend->drawItemCallback(drawInfo))
            return TRUE;
    }

    if(
        backend &&
        backend->commandCallback &&
        (message == WM_COMMAND)
    ) {
        const int controlId = LOWORD(wParam);
        const int notification = HIWORD(wParam);

        if(backend->commandCallback(controlId, notification))
            return 0;
    }

    if(message == WM_CLOSE)
    {
        if(backend->closeCallback)
            backend->closeCallback();

        DestroyWindow(window);

        return 0;
    }

    if(message == WM_DESTROY)
    {
        backend->m_window = nullptr;
        logger->print("Deleted launcher window");

        PostQuitMessage(0);

        return 0;
    }

    if(message == WM_NCDESTROY)
        SetWindowLongPtrA(window, GWLP_USERDATA, 0);

    return DefWindowProcA(
        window, message,
        wParam, lParam
    );
}

WinBackend::~WinBackend()
{
    Logger* logger = Logger::get_logger();

    clear_callbacks();
    close_window();

    if(m_classRegistered && m_instance)
    {
        if(UnregisterClassA(WINDOW_CLASS_NAME, m_instance))
            logger->print("Unregistered launcher window class");
        else
            logger->print("Failed to unregister launcher window class");
    }

    m_classRegistered = false;

    m_window = nullptr;
    m_instance = nullptr;
}
