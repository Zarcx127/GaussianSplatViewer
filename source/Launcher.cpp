#include "Launcher.hpp"

#include <dwmapi.h>
#include <shobjidl.h>

namespace
{
    constexpr const char* WINDOW_CLASS_NAME = "GaussianSplatLauncherWindow";
    constexpr const char* WINDOW_TITLE = "Gaussian Splat Launcher";

    constexpr int FILE_PATH_ID = 100;
    constexpr int SELECT_FILE_ID = 101;
    constexpr int OPEN_FILE_ID = 102;

    constexpr int CONTROL_MARGIN = 20;
    constexpr int CONTROL_HEIGHT = 24;
    constexpr int LABEL_TO_PATH_GAP = 6;
    constexpr int PATH_TO_BUTTON_GAP = 16;
    constexpr int BUTTON_WIDTH = 100;
    constexpr int BUTTON_GAP = 10;
    constexpr int FILE_PATH_WIDTH = 400;
    
    constexpr int CLIENT_WIDTH = (
        FILE_PATH_WIDTH + (CONTROL_MARGIN * 2)
    );

    constexpr int CLIENT_HEIGHT = (
        (CONTROL_MARGIN * 2) +
        (CONTROL_HEIGHT * 3) +
        LABEL_TO_PATH_GAP +
        PATH_TO_BUTTON_GAP
    );

    constexpr COLORREF BACKGROUND_COLOR = RGB(45, 45, 48);
    constexpr COLORREF CAPTION_COLOR = RGB(38, 38, 42);
    constexpr COLORREF CONTROL_COLOR = RGB(55, 55, 59);
    constexpr COLORREF BUTTON_COLOR = RGB(63, 63, 68);
    constexpr COLORREF BUTTON_PRESSED_COLOR = RGB(75, 75, 81);
    constexpr COLORREF BUTTON_DISABLED_COLOR = RGB(50, 50, 54);
    constexpr COLORREF BORDER_COLOR = RGB(88, 88, 94);
    constexpr COLORREF TEXT_COLOR = RGB(235, 235, 235);
    constexpr COLORREF DISABLED_TEXT_COLOR = RGB(145, 145, 150);

    constexpr DWORD WINDOW_STYLE = (
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX
    );

    int scale_dimension(int value, UINT dpi);
}

Launcher::Launcher(std::string* selectedFilePath)
{
    m_selectedFilePath = selectedFilePath;
}

bool Launcher::build()
{
    if(!m_selectedFilePath)
        return false;

    HRESULT result = CoInitializeEx(
        nullptr,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
    );

    if(FAILED(result))
        return false;

    m_comInitialized = true;

    m_instance = GetModuleHandleA(nullptr);
    if(!m_instance)
        return false;

    m_backgroundBrush = CreateSolidBrush(BACKGROUND_COLOR);
    m_controlBrush = CreateSolidBrush(CONTROL_COLOR);
    m_buttonBrush = CreateSolidBrush(BUTTON_COLOR);
    m_buttonPressedBrush = CreateSolidBrush(BUTTON_PRESSED_COLOR);
    m_buttonDisabledBrush = CreateSolidBrush(BUTTON_DISABLED_COLOR);
    m_borderBrush = CreateSolidBrush(BORDER_COLOR);

    if(
        !m_backgroundBrush ||
        !m_controlBrush ||
        !m_buttonBrush ||
        !m_buttonPressedBrush ||
        !m_buttonDisabledBrush ||
        !m_borderBrush
    ) {
        return false;
    }

    WNDCLASSEXA windowClass = {};

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = (CS_HREDRAW | CS_VREDRAW);
    windowClass.lpfnWndProc = Launcher::window_proc;
    windowClass.hInstance = m_instance;
    windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    windowClass.hbrBackground = m_backgroundBrush;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if(!RegisterClassExA(&windowClass))
        return false;

    m_window = CreateWindowExA(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WINDOW_STYLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CLIENT_WIDTH, CLIENT_HEIGHT,
        nullptr, nullptr,
        m_instance,
        this
    );

    if(!m_window)
        return false;

    const COLORREF captionColor = CAPTION_COLOR;
    const COLORREF captionTextColor = TEXT_COLOR;
    const COLORREF captionBorderColor = BORDER_COLOR;

    (void) DwmSetWindowAttribute(
        m_window, DWMWA_CAPTION_COLOR,
        &captionColor, sizeof(captionColor)
    );

    (void) DwmSetWindowAttribute(
        m_window, DWMWA_TEXT_COLOR,
        &captionTextColor, sizeof(captionTextColor)
    );

    (void) DwmSetWindowAttribute(
        m_window, DWMWA_BORDER_COLOR,
        &captionBorderColor, sizeof(captionBorderColor)
    );

    m_dpi = GetDpiForWindow(m_window);

    RECT windowRect = {
        0,
        0,
        scale_dimension(CLIENT_WIDTH, m_dpi),
        scale_dimension(CLIENT_HEIGHT, m_dpi)
    };

    if(!AdjustWindowRectExForDpi(
        &windowRect,
        WINDOW_STYLE,
        FALSE,
        0,
        m_dpi
    )) {
        return false;
    }

    const int windowWidth = windowRect.right - windowRect.left;
    const int windowHeight = windowRect.bottom - windowRect.top;

    HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if(!GetMonitorInfoA(monitor, &monitorInfo))
        return false;

    const int workWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
    const int workHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

    const int windowX = monitorInfo.rcWork.left + ((workWidth - windowWidth) / 2);
    const int windowY = monitorInfo.rcWork.top + ((workHeight - windowHeight) / 2);

    SetWindowPos(
        m_window, nullptr,
        windowX, windowY,
        windowWidth, windowHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    if(!build_controls())
        return false;

    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);

    return true;
}

LauncherResult Launcher::main_loop()
{
    m_result = {};

    MSG message = {};
    while(GetMessageA(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return m_result;
}

bool Launcher::build_controls()
{
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    m_filePathLabel = CreateWindowExA(
        0, "STATIC", "Selected File:",
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0,
        m_window, nullptr, m_instance, nullptr
    );

    m_filePathControl = CreateWindowExA(
        0, "EDIT", m_selectedFilePath->c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | ES_MULTILINE,
        0, 0, 0, 0,
        m_window, reinterpret_cast<HMENU>(FILE_PATH_ID), m_instance, nullptr
    );

    m_selectFileButton = CreateWindowExA(
        0, "BUTTON", "Select File",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 0, 0,
        m_window, reinterpret_cast<HMENU>(SELECT_FILE_ID), m_instance, nullptr
    );

    m_openFileButton = CreateWindowExA(
        0, "BUTTON", "Open",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 0, 0,
        m_window, reinterpret_cast<HMENU>(OPEN_FILE_ID), m_instance, nullptr
    );

    if(
        !m_filePathLabel ||
        !m_filePathControl ||
        !m_selectFileButton ||
        !m_openFileButton
    ) {
        return false;
    }
    
    SendMessageA(m_filePathLabel, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageA(m_filePathControl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageA(m_selectFileButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageA(m_openFileButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

    EnableWindow(m_openFileButton, !m_selectedFilePath->empty());

    layout_controls();

    return true;
}

bool Launcher::select_file()
{
    IFileOpenDialog* fileDialog = nullptr;

    HRESULT result = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&fileDialog)
    );

    if(FAILED(result))
        return false;

    COMDLG_FILTERSPEC fileTypes[] = {
        { L"PLY Files (*.ply)", L"*.ply" },
        { L"All Files (*.*)", L"*.*" }
    };

    result = fileDialog->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);

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
            MB_OK | MB_ICONERROR
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

    int filePathLength = WideCharToMultiByte(
        CP_ACP,
        0,
        wideFilePath,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if(filePathLength <= 0)
    {
        CoTaskMemFree(wideFilePath);
        return false;
    }

    std::string filePath(filePathLength, '\0');

    int convertedLength = WideCharToMultiByte(
        CP_ACP,
        0,
        wideFilePath,
        -1,
        filePath.data(),
        filePathLength,
        nullptr,
        nullptr
    );

    CoTaskMemFree(wideFilePath);

    if(convertedLength <= 0)
        return false;

    filePath.resize(convertedLength - 1);

    *m_selectedFilePath = filePath;

    SetWindowTextA(m_filePathControl, m_selectedFilePath->c_str());
    EnableWindow(m_openFileButton, TRUE);

    return true;
}

void Launcher::layout_controls()
{
    const int margin = scale_dimension(CONTROL_MARGIN, m_dpi);
    const int controlHeight = scale_dimension(CONTROL_HEIGHT, m_dpi);
    const int labelToPathGap = scale_dimension(LABEL_TO_PATH_GAP, m_dpi);
    const int pathToButtonGap = scale_dimension(PATH_TO_BUTTON_GAP, m_dpi);
    const int buttonWidth = scale_dimension(BUTTON_WIDTH, m_dpi);
    const int buttonGap = scale_dimension(BUTTON_GAP, m_dpi);
    const int filePathWidth = scale_dimension(FILE_PATH_WIDTH, m_dpi);

    const int filePathY =
        margin + controlHeight + labelToPathGap;

    const int buttonY =
        filePathY + controlHeight + pathToButtonGap;

    const int openButtonX =
        margin + filePathWidth - buttonWidth;

    const int selectFileButtonX =
        openButtonX - buttonGap - buttonWidth;

    MoveWindow(
        m_filePathLabel,
        margin, margin,
        filePathWidth, controlHeight,
        TRUE
    );

    MoveWindow(
        m_filePathControl,
        margin, filePathY,
        filePathWidth, controlHeight,
        TRUE
    );

    layout_file_path_text();

    MoveWindow(
        m_selectFileButton,
        selectFileButtonX, buttonY,
        buttonWidth, controlHeight,
        TRUE
    );

    MoveWindow(
        m_openFileButton,
        openButtonX, buttonY,
        buttonWidth, controlHeight,
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

    const int textHeight = textMetrics.tmHeight;
    const int verticalMargin = (controlRect.bottom - textHeight) / 2;

    RECT textRect = {
        scale_dimension(6, m_dpi),
        verticalMargin,
        controlRect.right - scale_dimension(6, m_dpi),
        verticalMargin + textHeight
    };

    SendMessageA(
        m_filePathControl,
        EM_SETRECT,
        0,
        reinterpret_cast<LPARAM>(&textRect)
    );
}

void Launcher::draw_button(const DRAWITEMSTRUCT* drawInfo)
{
    HBRUSH backgroundBrush = m_buttonBrush;
    COLORREF textColor = TEXT_COLOR;

    if(drawInfo->itemState & ODS_DISABLED)
    {
        backgroundBrush = m_buttonDisabledBrush;
        textColor = DISABLED_TEXT_COLOR;
    }
    else if(drawInfo->itemState & ODS_SELECTED)
    {
        backgroundBrush = m_buttonPressedBrush;
    }

    FillRect(drawInfo->hDC, &drawInfo->rcItem, backgroundBrush);
    FrameRect(drawInfo->hDC, &drawInfo->rcItem, m_borderBrush);

    char text[64] = {};
    GetWindowTextA(drawInfo->hwndItem, text, sizeof(text));

    RECT textRect = drawInfo->rcItem;

    if(drawInfo->itemState & ODS_SELECTED)
        OffsetRect(&textRect, 1, 1);

    SetBkMode(drawInfo->hDC, TRANSPARENT);
    SetTextColor(drawInfo->hDC, textColor);

    DrawTextA(
        drawInfo->hDC, text, -1, &textRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
}

LRESULT CALLBACK Launcher::window_proc(
    HWND window, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
) {
    Launcher* launcher = reinterpret_cast<Launcher*>(
        GetWindowLongPtrA(window, GWLP_USERDATA)
    );

    if(message == WM_NCCREATE)
    {
        CREATESTRUCTA* createInfo = reinterpret_cast<CREATESTRUCTA*>(lParam);

        launcher = reinterpret_cast<Launcher*>(createInfo->lpCreateParams);

        SetWindowLongPtrA(
            window,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(launcher)
        );

        if(launcher)
            launcher->m_window = window;
    }

    if((message == WM_DPICHANGED) && launcher)
    {
        launcher->m_dpi = HIWORD(wParam);

        RECT* suggestedRect = reinterpret_cast<RECT*>(lParam);

        SetWindowPos(
            window,
            nullptr,
            suggestedRect->left,
            suggestedRect->top,
            suggestedRect->right - suggestedRect->left,
            suggestedRect->bottom - suggestedRect->top,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        launcher->layout_controls();

        return 0;
    }

    if((message == WM_CTLCOLORSTATIC) && launcher)
    {
        HDC deviceContext = reinterpret_cast<HDC>(wParam);
        HWND control = reinterpret_cast<HWND>(lParam);

        SetTextColor(deviceContext, TEXT_COLOR);

        if(control == launcher->m_filePathControl)
        {
            SetBkColor(deviceContext, CONTROL_COLOR);
            return reinterpret_cast<LRESULT>(launcher->m_controlBrush);
        }

        SetBkMode(deviceContext, TRANSPARENT);
        return reinterpret_cast<LRESULT>(launcher->m_backgroundBrush);
    }

    if((message == WM_DRAWITEM) && launcher)
    {
        DRAWITEMSTRUCT* drawInfo = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);

        if(
            (drawInfo->CtlID == SELECT_FILE_ID) ||
            (drawInfo->CtlID == OPEN_FILE_ID)
        ) {
            launcher->draw_button(drawInfo);
            return TRUE;
        }
    }

    if((message == WM_COMMAND) && launcher)
    {
        const int controlId = LOWORD(wParam);
        const int notification = HIWORD(wParam);

        if(notification == BN_CLICKED)
        {
            if(controlId == SELECT_FILE_ID)
            {
                (void) launcher->select_file();
                return 0;
            }

            if(controlId == OPEN_FILE_ID)
            {
                if(launcher->m_selectedFilePath->empty())
                    return 0;

                launcher->m_result.action = LauncherAction::OpenViewer;
                launcher->m_result.filePath = launcher->m_selectedFilePath->c_str();

                DestroyWindow(window);

                return 0;
            }
        }
    }

    if(message == WM_CLOSE)
    {
        DestroyWindow(window);
        return 0;
    }


    if(message == WM_DESTROY)
    {
        if(launcher)
            launcher->m_window = nullptr;

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(window, message, wParam, lParam);
}

Launcher::~Launcher()
{
    if(m_backgroundBrush) DeleteObject(m_backgroundBrush);
    if(m_controlBrush) DeleteObject(m_controlBrush);
    if(m_buttonBrush) DeleteObject(m_buttonBrush);
    if(m_buttonPressedBrush) DeleteObject(m_buttonPressedBrush);
    if(m_buttonDisabledBrush) DeleteObject(m_buttonDisabledBrush);
    if(m_borderBrush) DeleteObject(m_borderBrush);

    if(m_window) 
        DestroyWindow(m_window);
    
    if(m_comInitialized) 
        CoUninitialize();
    
    if(m_instance) 
        UnregisterClassA(WINDOW_CLASS_NAME, m_instance);

    m_instance = nullptr;

    m_window = nullptr;
    m_filePathLabel = nullptr;
    m_filePathControl = nullptr;
    m_selectFileButton = nullptr;
    m_openFileButton = nullptr;

    m_backgroundBrush = nullptr;
    m_controlBrush = nullptr;
    m_buttonBrush = nullptr;
    m_buttonPressedBrush = nullptr;
    m_buttonDisabledBrush = nullptr;
    m_borderBrush = nullptr;

    m_comInitialized = false;
    m_selectedFilePath = nullptr;
}

namespace 
{
    int scale_dimension(int value, UINT dpi)
    {
        return MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI);
    }
}
