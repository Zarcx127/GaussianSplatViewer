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

#include "Launcher.hpp"

#include <shobjidl.h>

#include "logging/Logger.hpp"

namespace
{
    constexpr int FILE_PATH_ID = 100;
    constexpr int SELECT_FILE_ID = 101;
    constexpr int OPEN_FILE_ID = 102;

    constexpr int CONTROL_MARGIN = 20;
    constexpr int CONTROL_HEIGHT = 24;
    constexpr int LABEL_TO_FILE_PATH_GAP = 6;
    constexpr int FILE_PATH_TO_BUTTON_GAP = 16;
    constexpr int FILE_PATH_WIDTH = 400;
    constexpr int BUTTON_WIDTH = 100;
    constexpr int BUTTON_GAP = 10;

    constexpr COLORREF BACKGROUND_COLOR = RGB(45, 45, 48);
    constexpr COLORREF CAPTION_COLOR = RGB(38, 38, 42);
    constexpr COLORREF CONTROL_COLOR = RGB(55, 55, 59);
    constexpr COLORREF BUTTON_COLOR = RGB(63, 63, 68);
    constexpr COLORREF BUTTON_PRESSED_COLOR = RGB(75, 75, 81);
    constexpr COLORREF BUTTON_DISABLED_COLOR = RGB(50, 50, 54);
    constexpr COLORREF BORDER_COLOR = RGB(88, 88, 94);
    constexpr COLORREF TEXT_COLOR = RGB(235, 235, 235);
    constexpr COLORREF DISABLED_TEXT_COLOR = RGB(145, 145, 150);
}

Launcher::Launcher(WinBackend& backend, std::filesystem::path& selectedFilePath) 
{
    m_backend = &backend;
    m_selectedFilePath = &selectedFilePath;

    m_window = m_backend->get_window();

    m_backend->eraseBackgroundCallback =
        [this] (HDC deviceContext)->bool {
            return erase_background_callback(deviceContext);
        };

    m_backend->dpiChangedCallback = 
        [this] (UINT dpi)->void {
            dpi_changed_callback(dpi);
        };

    m_backend->controlColorCallback = 
        [this] (HDC deviceContext, HWND control)->HBRUSH {
            return control_color_callback(deviceContext, control);
        };

    m_backend->drawItemCallback = 
        [this] (const DRAWITEMSTRUCT* drawInfo)->bool {
            return draw_item_callback(drawInfo);
        };

    m_backend->commandCallback = 
        [this] (int controlId, int notification)->bool {
            return command_callback(controlId, notification);
        };

    m_backend->closeCallback = 
        [this] ()->void {
            m_result = LauncherResult::ExitApplication;
        };
}

bool Launcher::build()
{
    Logger* logger = Logger::get_logger();

    if(!m_window)
    {
        logger->print("Cannot create launcher without a Win32 window");
        return false;
    }

    HRESULT result = CoInitializeEx(
        nullptr,
        (COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)
    );

    if(FAILED(result))
    {
        logger->print("Failed to initialize launcher COM");
        return false;
    }

    m_comInitialized = true;

    logger->print("Initialized launcher COM");

    if(!create_brush(m_backgroundBrush, BACKGROUND_COLOR))
        return false;

    if(!create_brush(m_controlBrush, CONTROL_COLOR))
        return false;

    if(!create_brush(m_buttonBrush, BUTTON_COLOR))
        return false;

    if(!create_brush(m_buttonPressedBrush, BUTTON_PRESSED_COLOR))
        return false;

    if(!create_brush(m_buttonDisabledBrush, BUTTON_DISABLED_COLOR))
        return false;

    if(!create_brush(m_borderBrush, BORDER_COLOR))
        return false;

    if(!m_backend->set_window_colors(
        CAPTION_COLOR, TEXT_COLOR,
        BORDER_COLOR
    )) {
        logger->print("Failed to configure launcher window colors");
        return false;
    }

    logger->print("Configured launcher window colors");

    m_dpi = m_backend->get_dpi();

    if(!build_controls())
    {
        logger->print("Failed to build launcher controls");
        return false;
    }

    m_backend->show_window();

    logger->print("Created launcher interface");

    return true;
}

LauncherResult Launcher::main_loop()
{
    m_result = LauncherResult::Running;
    while(m_backend->wait_events());
    
    return m_result;
}

bool Launcher::build_controls()
{
    Logger* logger = Logger::get_logger();

    HINSTANCE instance = reinterpret_cast<HINSTANCE>(
        GetWindowLongPtrA(m_window, GWLP_HINSTANCE)
    );

    HFONT font = static_cast<HFONT>(
        GetStockObject(DEFAULT_GUI_FONT)
    );

    const wchar_t* selectedFilePath = m_selectedFilePath->c_str();

    m_filePathLabel = CreateWindowExA(
        0, "STATIC", "Selected File:",
        (WS_CHILD | WS_VISIBLE),
        0, 0,
        0, 0,
        m_window, nullptr,
        instance, nullptr
    );

    m_filePathControl = CreateWindowExW(
        0, L"EDIT", selectedFilePath,
        (
            WS_CHILD |
            WS_VISIBLE |
            ES_AUTOHSCROLL |
            ES_READONLY |
            ES_MULTILINE
        ),
        0, 0,
        0, 0,
        m_window, reinterpret_cast<HMENU>(FILE_PATH_ID),
        instance, nullptr
    );

    m_selectFileButton = CreateWindowExA(
        0, "BUTTON", "Select .ply file",
        (WS_CHILD | WS_VISIBLE | BS_OWNERDRAW),
        0, 0,
        0, 0,
        m_window, reinterpret_cast<HMENU>(SELECT_FILE_ID),
        instance, nullptr
    );

    m_openFileButton = CreateWindowExA(
        0, "BUTTON", "Open",
        (WS_CHILD | WS_VISIBLE | BS_OWNERDRAW),
        0, 0,
        0, 0,
        m_window, reinterpret_cast<HMENU>(OPEN_FILE_ID),
        instance, nullptr
    );

    if(
        !m_filePathLabel ||
        !m_filePathControl ||
        !m_selectFileButton ||
        !m_openFileButton
    ) {
        logger->print("Failed to create launcher controls");
        return false;
    }

    SendMessageA(
        m_filePathLabel, WM_SETFONT,
        reinterpret_cast<WPARAM>(font), TRUE
    );

    SendMessageA(
        m_filePathControl, WM_SETFONT,
        reinterpret_cast<WPARAM>(font), TRUE
    );

    SendMessageA(
        m_selectFileButton, WM_SETFONT,
        reinterpret_cast<WPARAM>(font), TRUE
    );

    SendMessageA(
        m_openFileButton, WM_SETFONT,
        reinterpret_cast<WPARAM>(font), TRUE
    );

    EnableWindow(m_openFileButton, !m_selectedFilePath->empty());

    layout_controls();

    logger->print("Created launcher controls");

    return true;
}

bool Launcher::select_file()
{
    IFileOpenDialog* fileDialog = nullptr;

    HRESULT result = CoCreateInstance(
        CLSID_FileOpenDialog, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&fileDialog)
    );

    if(FAILED(result))
        return false;

    COMDLG_FILTERSPEC fileTypes[] = {
        { L"PLY Files (*.ply)", L"*.ply" },
        { L"All Files (*.*)", L"*.*" }
    };

    result = fileDialog->SetFileTypes(
        ARRAYSIZE(fileTypes),
        fileTypes
    );

    FILEOPENDIALOGOPTIONS options = {};
    if(SUCCEEDED(result))
        result = fileDialog->GetOptions(&options);

    if(SUCCEEDED(result))
    {
        result = fileDialog->SetOptions(
            options |
            FOS_FORCEFILESYSTEM |
            FOS_FILEMUSTEXIST |
            FOS_PATHMUSTEXIST |
            FOS_NOCHANGEDIR
        );
    }

    if(SUCCEEDED(result))
        result = fileDialog->Show(m_window);

    if(result == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        fileDialog->Release();
        return false;
    }

    if(FAILED(result))
    {
        fileDialog->Release();

        MessageBoxA(
            m_window,
            "Failed to open the file selection dialog.",
            "File Selection Error",
            (MB_OK | MB_ICONERROR)
        );

        return false;
    }

    IShellItem* selectedItem = nullptr;

    result = fileDialog->GetResult(&selectedItem);
    fileDialog->Release();

    if(FAILED(result) || !selectedItem)
        return false;

    PWSTR wideFilePath = nullptr;

    result = selectedItem->GetDisplayName(
        SIGDN_FILESYSPATH,
        &wideFilePath
    );

    selectedItem->Release();

    if(FAILED(result) || !wideFilePath)
        return false;

    *m_selectedFilePath = std::filesystem::path(wideFilePath);

    CoTaskMemFree(wideFilePath);

    SetWindowTextW(
        m_filePathControl,
        m_selectedFilePath->c_str()
    );

    EnableWindow(m_openFileButton, TRUE);

    return true;
}

void Launcher::layout_controls()
{
    int controlMargin = scale_dimension(CONTROL_MARGIN, m_dpi);
    int controlHeight = scale_dimension(CONTROL_HEIGHT, m_dpi);

    int labelToFilePathGap = scale_dimension(LABEL_TO_FILE_PATH_GAP, m_dpi);
    int filePathToButtonGap = scale_dimension(FILE_PATH_TO_BUTTON_GAP, m_dpi);

    int filePathWidth = scale_dimension(FILE_PATH_WIDTH, m_dpi);
    int buttonWidth = scale_dimension(BUTTON_WIDTH, m_dpi);
    int buttonHeight = controlHeight;
    int buttonGap = scale_dimension(BUTTON_GAP, m_dpi);

    int filePathY = (controlMargin + controlHeight + labelToFilePathGap);
    int buttonY = (filePathY + controlHeight + filePathToButtonGap);

    int openFileButtonX = (controlMargin + filePathWidth - buttonWidth);
    int selectFileButtonX = (openFileButtonX - buttonGap - buttonWidth);

    MoveWindow(
        m_filePathLabel,
        controlMargin, controlMargin,
        filePathWidth, controlHeight,
        TRUE
    );

    MoveWindow(
        m_filePathControl,
        controlMargin, filePathY,
        filePathWidth, controlHeight,
        TRUE
    );

    layout_file_path_text();

    MoveWindow(
        m_selectFileButton,
        selectFileButtonX, buttonY,
        buttonWidth, buttonHeight,
        TRUE
    );

    MoveWindow(
        m_openFileButton,
        openFileButtonX, buttonY,
        buttonWidth, buttonHeight,
        TRUE
    );
}

void Launcher::layout_file_path_text()
{
    RECT controlRect = {};
    GetClientRect(m_filePathControl, &controlRect);

    HDC deviceContext = GetDC(m_filePathControl);

    HFONT font = reinterpret_cast<HFONT>(
        SendMessageA(m_filePathControl, WM_GETFONT, 0, 0)
    );

    HFONT oldFont = reinterpret_cast<HFONT>(
        SelectObject(deviceContext, font)
    );

    TEXTMETRICA textMetrics = {};
    GetTextMetricsA(deviceContext, &textMetrics);

    SelectObject(deviceContext, oldFont);
    ReleaseDC(m_filePathControl, deviceContext);

    int textHeight = textMetrics.tmHeight;
    int horizontalMargin = scale_dimension(6, m_dpi);
    int verticalMargin = ((controlRect.bottom - textHeight) / 2);

    RECT textRect = {
        horizontalMargin,
        verticalMargin,
        (controlRect.right - horizontalMargin),
        (verticalMargin + textHeight)
    };

    SendMessageA(
        m_filePathControl, EM_SETRECT,
        0, reinterpret_cast<LPARAM>(&textRect)
    );
}

void Launcher::draw_button(const DRAWITEMSTRUCT* drawInfo)
{
    HBRUSH buttonBrush = m_buttonBrush;
    COLORREF buttonTextColor = TEXT_COLOR;

    if((drawInfo->itemState & ODS_DISABLED) != 0)
    {
        buttonBrush = m_buttonDisabledBrush;
        buttonTextColor = DISABLED_TEXT_COLOR;
    }
    else if((drawInfo->itemState & ODS_SELECTED) != 0)
    {
        buttonBrush = m_buttonPressedBrush;
    }

    FillRect(drawInfo->hDC, &drawInfo->rcItem, buttonBrush);
    FrameRect(drawInfo->hDC, &drawInfo->rcItem, m_borderBrush);

    char buttonText[64] = {};

    GetWindowTextA(
        drawInfo->hwndItem,
        buttonText, sizeof(buttonText)
    );

    RECT buttonTextRect = drawInfo->rcItem;
    if((drawInfo->itemState & ODS_SELECTED) != 0)
        OffsetRect(&buttonTextRect, 1, 1);

    SetBkMode(drawInfo->hDC, TRANSPARENT);
    SetTextColor(drawInfo->hDC, buttonTextColor);

    DrawTextA(
        drawInfo->hDC,
        buttonText, -1,
        &buttonTextRect,
        (DT_CENTER | DT_VCENTER | DT_SINGLELINE)
    );
}

void Launcher::dpi_changed_callback(UINT dpi)
{
    m_dpi = dpi;

    if(m_filePathLabel)
        layout_controls();
}

bool Launcher::erase_background_callback(HDC deviceContext)
{
    RECT clientRect {};

    if(!GetClientRect(m_window, &clientRect))
        return false;

    return (
        FillRect(
            deviceContext,
            &clientRect,
            m_backgroundBrush
        ) != 0
    );
}

HBRUSH Launcher::control_color_callback(
    HDC deviceContext,
    HWND control
) {
    SetTextColor(deviceContext, TEXT_COLOR);

    if(control == m_filePathControl)
    {
        SetBkColor(deviceContext, CONTROL_COLOR);
        return m_controlBrush;
    }

    SetBkMode(deviceContext, TRANSPARENT);

    return m_backgroundBrush;
}

bool Launcher::draw_item_callback(const DRAWITEMSTRUCT* drawInfo)
{
    if(!drawInfo)
        return false;

    if(
        (drawInfo->CtlID != SELECT_FILE_ID) &&
        (drawInfo->CtlID != OPEN_FILE_ID)
    ) {
        return false;
    }

    draw_button(drawInfo);

    return true;
}

bool Launcher::command_callback(int controlId, int notification)
{
    if(notification != BN_CLICKED)
        return false;

    if(controlId == SELECT_FILE_ID)
    {
        (void) select_file();
        return true;
    }

    if(controlId == OPEN_FILE_ID)
    {
        if(m_selectedFilePath->empty())
            return true;

        m_result = LauncherResult::OpenViewer;
        m_backend->close_window();

        return true;
    }

    return false;
}

bool Launcher::create_brush(HBRUSH& brush, COLORREF color)
{
    Logger* logger = Logger::get_logger();

    brush = CreateSolidBrush(color);
    if(!brush)
    {
        logger->print("Failed to create launcher brush");
        return false;
    }

    logger->print("Created launcher brush");

    m_brushDeletionQueue.push_back(
        [logger, &brush] ()->void {
            DeleteObject(brush);
            brush = nullptr;

            logger->print("Deleted launcher brush");
        }
    );

    return true;
}

int Launcher::scale_dimension(int value, UINT dpi)
{
    return MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI);
}

Launcher::~Launcher()
{
    Logger* logger = Logger::get_logger();
    
    m_backend->clear_callbacks();

    while(!m_brushDeletionQueue.empty())
    {
        (m_brushDeletionQueue.back())();
        m_brushDeletionQueue.pop_back();
    }

    if(m_comInitialized)
    {
        CoUninitialize();
        logger->print("Uninitialized launcher COM");
    }

    m_backend = nullptr;
    m_window = nullptr;

    m_filePathLabel = nullptr;
    m_filePathControl = nullptr;
    m_selectFileButton = nullptr;
    m_openFileButton = nullptr;

    m_comInitialized = false;
    m_selectedFilePath = nullptr;

    logger->print("Deleted launcher interface");
}

