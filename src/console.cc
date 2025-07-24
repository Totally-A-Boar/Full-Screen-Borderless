// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#include "console.h"

#include "error.h"
#include "fsb_string.h"
#include <colors/colors.hpp>

#include <cassert>
#include <fcntl.h>
#include <io.h>
#include <sstream>
#include <string>

namespace fsb {
Console::Console(const Config& config)
    : clear_console_(false),
      refresh_line_(0),
      menu_section_(true),
      index_section_0_(0),
      index_section_1_x_(0),
      index_section_1_y_(0),
      config_(config) {
    const auto kConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (kConsoleHandle == INVALID_HANDLE_VALUE) {
        constexpr std::string_view kActionDesc = "setup the console for UTF-8 I/O.";
        constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::Console";
        constexpr std::string_view kExportedFunctionName = "Kernel32.dll!GetStdHandle";
        constexpr int kReturnValue = -1;
        WIN32_ERROR(kActionDesc, kQualifiedName, kExportedFunctionName, kReturnValue);
        std::exit(FSB_CONSOLE_INIT_FAILURE);
    }

    // Calls can only fail if the handle is invalid
    CONSOLE_CURSOR_INFO info;
    static_cast<void>(GetConsoleCursorInfo(kConsoleHandle, &info));
    info.bVisible = false;
    static_cast<void>(SetConsoleCursorInfo(kConsoleHandle, &info));

    if (!SetConsoleOutputCP(CP_UTF8) || !SetConsoleCP(CP_UTF8)) {
        constexpr std::string_view kActionDesc = "setup the console active code page.";
        constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::Console";
        constexpr std::string_view kExportedFunctionName = "Kernel32.dll!SetConsoleOutputCP";
        constexpr int kReturnValue = 0;
        WIN32_ERROR(kActionDesc, kQualifiedName, kExportedFunctionName, kReturnValue);
        std::exit(FSB_CONSOLE_INIT_FAILURE);
    }

    // Note: _O_TEXT and _O_U8TEXT, while fundamentally the same and have the same value serve two
    // different functions and greatly impact the console output.
    // The C Runtime will differentiate between the two internally.
    // _O_U8TEXT translates UTF-16 (wcout) output to UTF-8, however, this will mangle UTF-8 output
    // and result in mojibake output.
    // Return value is ignored because _setmode returns the previous translation mode of the file.
    static_cast<void>(_setmode(_fileno(stdout), _O_TEXT));
    static_cast<void>(_setmode(_fileno(stderr), _O_TEXT));
    static_cast<void>(_setmode(_fileno(stdin), _O_TEXT));

    static_cast<void>(SetConsoleTitleW(L"Full Screen Borderless"));
}

Console::~Console() {
    const auto kConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (kConsoleHandle == INVALID_HANDLE_VALUE) {
        constexpr std::string_view kActionDescription = "uninitialize the console.";
        constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::Console";
        constexpr std::string_view kExportedFunctionName = "Kernel32.dll!GetStdHandle";
        constexpr int kReturnValue = -1;
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedFunctionName, kReturnValue);
        std::exit(FSB_CONSOLE_UNINIT_FAILURE);
    }

    CONSOLE_CURSOR_INFO cursorInfo;

    static_cast<void>(GetConsoleCursorInfo(kConsoleHandle, &cursorInfo));
    cursorInfo.bVisible = true;
    static_cast<void>(SetConsoleCursorInfo(kConsoleHandle, &cursorInfo));
}

bool Console::GetWindowAttributes(HWND window_handle, WindowAttributes* window_attributes) {
    if (window_handle == nullptr || !IsWindow(window_handle)) {
        return false;
    }

    const bool kIsWindowVisible = IsWindowVisible(window_handle);

    const bool kIsWindowEnabled = IsWindowEnabled(window_handle);

    WindowState window_state = WindowState::Normal;
    WINDOWPLACEMENT window_placement;
    window_placement.length = sizeof(window_placement);

    if (GetWindowPlacement(window_handle, &window_placement)) {
        // showCmd is the property used to determine window state.
        // For example, nCmdShow in the WinMain entry point is the state to show the window in.
        // I used to think it meant whether to show the CMD window or not, however it means show
        // command not show command prompt.
        // It's only real use is this and ShowWindow which is the prefix in the macro SW means.
        // The values we need here are SW_SHOWMAXIMIZED, SW_SHOWMINIMIZED, SW_SHOWNORMAL and/or
        // SW_RESTORE (these two mean the same).
        switch (window_placement.showCmd) {
            case SW_SHOWMAXIMIZED:
                window_state = WindowState::Maximized;
                break;
            case SW_SHOWMINIMIZED:
                window_state = WindowState::Minimized;
                break;
            default:
                window_state = WindowState::Normal;
                break;
        }
    } else {
        return false;
    }

    window_attributes->is_enabled_ = kIsWindowEnabled;
    window_attributes->is_visible_ = kIsWindowVisible;
    window_attributes->state_ = window_state;

    return true;
}

bool Console::GetWindowMetrics(HWND window_handle, WindowMetrics* window_metrics) {
    if (window_handle == nullptr || !IsWindow(window_handle)) {
        return false;
    }

    RECT window_rect;
    if (!GetWindowRect(window_handle, &window_rect)) {
        return false;
    }

    int x, y, width, height;
    x = window_rect.left;
    y = window_rect.top;
    width = window_rect.right - window_rect.left;
    height = window_rect.bottom - window_rect.top;

    auto font_handle = reinterpret_cast<HFONT>(SendMessageTimeoutW(window_handle, WM_GETFONT,
        0, 0, SMTO_ABORTIFHUNG, 100, nullptr));

    std::string font_name = "";
    uint32_t font_size = 0;

    LOGFONT log_font = {};
    if (font_handle != nullptr) {
        if (GetObjectW(font_handle, sizeof(LOGFONT), &log_font)) {
            std::wstring buffer = log_font.lfFaceName;
            font_name = Utf16ToUtf8(buffer);

            HDC device_context = GetDC(window_handle);
            int dpi = GetDeviceCaps(device_context, LOGPIXELSY);
            ReleaseDC(window_handle, device_context);

            // Conversion: font size (in pixels) = lfHeight * 72 / DPI.
            // 72 in this case is representative of 1 point (pixel) being 1/72 of an inch which is
            // divided by DPI in case the dots per inch is more than 1/72.
            if (log_font.lfHeight < 0) {
                // Normally, negative height means character height in logical units.
                font_size = static_cast<uint32_t>(-log_font.lfHeight * 72 / dpi);
            } else {
                // While uncommon, positive height is possible.
                // Consider calling GetTextMetrics if this conversion is buggy.
                font_size = static_cast<uint32_t>(log_font.lfHeight * 72 / dpi);
            }
        }
    } else {
        // TODO (jhowell728): Implement logging calls.
        font_name = "None";
        font_size = 0;
    }

    const uint32_t kStyle = static_cast<uint32_t>(GetWindowLongPtrW(window_handle,
        GWL_STYLE));
    const uint32_t kExStyle = static_cast<uint32_t>(GetWindowLongPtrW(window_handle,
        GWL_EXSTYLE));

    const SizeVec2 kPosition = {x, y};
    const SizeVec2 kSize = {width, height};

    window_metrics->position_ = kPosition;
    window_metrics->size_ = kSize;
    window_metrics->font_name_ = font_name;
    window_metrics->font_size_ = font_size;
    window_metrics->style_ = kStyle;
    window_metrics->ex_style_ = kExStyle;

    return true;
}

std::string Console::GetProcessFileName(uint32_t process_id) {
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
        false, process_id);
    if (!process_handle) {
        // TODO(jhowell728): Come up with a better unknown file name value
        return "???";
    }

    wchar_t process_file_name[MAX_PATH];
    uint32_t size = MAX_PATH;

    if (QueryFullProcessImageNameW(process_handle, 0, process_file_name,
        reinterpret_cast<DWORD*>(&size))) {
        CloseHandle(process_handle);
        return Utf16ToUtf8(process_file_name);
    }

    CloseHandle(process_handle);
    return "???";
}

void Console::ClearConsole() {
    // Modern Windows consoles (post UTF-16 implementation) don't interpret ANSI escape codes and
    // must be either set via attributes or ANSI codes must be enabled
    auto console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        constexpr std::string_view kActionDescription = "clear the console.";
        constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::ClearConsole";
        constexpr std::string_view kExportedFunctionName = "Kernel32.dll!GetStdHandle";
        constexpr int kReturnCode = -1;
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedFunctionName, kReturnCode);
        return;
    }

    DWORD mode = 0;
    if (GetConsoleMode(console_handle, &mode)) {
        if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            SetConsoleMode(console_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    std::cout << "\033c[2J\033[H" << std::flush;
}

// TODO(jhowell728): clean up logic and add better error codes
int Console::EnumWindowsCallback(HWND window_handle, LPARAM message_param) {
    if (window_handle == nullptr || !IsWindow(window_handle)) {
        return 1;
    }

    auto console = reinterpret_cast<Console*>(message_param);

    WindowAttributes window_attributes = {};
    if (!GetWindowAttributes(window_handle, &window_attributes)) {
        constexpr std::string_view kActionDescription = "get the attributes for a window";
        constexpr std::string_view kQualifiedName =
            "console.cc::fsb::Console::EnumWindowsCallback";
        constexpr std::string_view kExportedOperationName = "fsb.exe!GetWindowAttributes";
        const auto kReturnCode = static_cast<uint32_t>(GetLastError());
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
    }

    if (!window_attributes.is_visible_ && console->config_.hide_hidden_windows_) {
        return 1;
    }

    WindowMetrics window_metrics = {};
    if (!GetWindowMetrics(window_handle, &window_metrics)) {
        constexpr std::string_view kActionDescription = "get the metrics for a window";
        constexpr std::string_view kQualifiedName =
            "console.cc::fsb::Console::EnumWindowsCallback";
        constexpr std::string_view kExportedOperationName = "fsb.exe!GetWindowMetrics";
        const auto kReturnCode = static_cast<uint32_t>(GetLastError());
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
    }

    if (window_metrics.ex_style_ & WS_EX_TOOLWINDOW) {
        return 1;
    }

    uint32_t process_id = 0;
    if (GetWindowThreadProcessId(window_handle, reinterpret_cast<DWORD*>(&process_id)) == 0) {
        constexpr std::string_view kActionDescription = "get the process ID for a window.";
        constexpr std::string_view kQualifiedName =
            "console.cc::fsb::Console::EnumWindowsCallback";
        constexpr std::string_view kExportedFunctionName = "User32.dll!GetWindowThreadProcessId";
        constexpr int kReturnCode = 0;
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedFunctionName, kReturnCode);
        return 1;
    }

    wchar_t title_buffer[256];
    if (GetWindowTextW(window_handle, title_buffer, std::size(title_buffer)) == 0) {
        if (const auto kReturnCode = static_cast<uint32_t>(GetLastError());
            kReturnCode != 0 && kReturnCode != ERROR_SEM_NOT_FOUND
            && kReturnCode != ERROR_ACCESS_DENIED) {
            constexpr std::string_view kActionDescription = "get the title of a window.";
            constexpr std::string_view kQualifiedName =
                "console.cc::fsb::Console::EnumWindowsCallback";
            constexpr std::string_view kExportedOperationName = "User32.dll!GetWindowTextW";
            WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
        }
    }

    if (title_buffer[0] == L'\0' && console->config_.hide_blank_title_windows_) {
        return 1;
    }

    wchar_t class_buffer[256];
    if (GetClassNameW(window_handle, class_buffer, std::size(class_buffer)) == 0) {
        if (const auto kReturnCode = static_cast<uint32_t>(GetLastError());
            kReturnCode != 0) {
            constexpr std::string_view kActionDescription = "get the class name of a window.";
            constexpr std::string_view kQualifiedName =
                "console.cc::fsb::Console::EnumWindowsCallback";
            constexpr std::string_view kExportedOperationName = "User32.dll!GetClassNameW";
            WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
        }
    }

    std::string title;
    if (title_buffer[0] == L'\0') {
        title = "";
    } else {
        title = Utf16ToUtf8(title_buffer);
    }

    std::string class_name = Utf16ToUtf8(class_buffer);
    std::string file_name = GetProcessFileName(process_id);

    ProcessData process_data = {};
    process_data.attributes_ = window_attributes;
    process_data.class_name_ = class_name;
    process_data.file_name_ = file_name;
    process_data.metrics_ = window_metrics;
    process_data.process_id_ = process_id;
    process_data.title_ = title;
    process_data.window_handle_ = window_handle;

    console->windows_.push_back(process_data);

    return 1;
}

void Console::RefreshWindows() {
    windows_.clear();
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this));
}

void Console::DispatchKeyPress(char key, ProcessData* process_data) {
    UNREFERENCED_PARAMETER(process_data);
    switch (toupper(key)) {
        case VK_ESCAPE:
        case 'Q':
            std::exit(0);
            break;
        case 'R':
            RefreshWindows();
            break;
        case VK_RETURN:
            menu_section_ = true;
            break;
    }
}

void Console::ShowMenu() {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        constexpr std::string_view kActionDescription = "get the console handle.";
        constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::ShowMenu";
        constexpr std::string_view kExportedOperationName = "Kernel32.dll!GetStdHandle";
        constexpr int kReturnCode = -1;
        WIN32_FAILFAST(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
    }

    RefreshWindows();

    while (true) {
        assert(!windows_.empty());
        bool cursor_result = SetConsoleCursorPosition(console_handle,
            {0, 0});
        assert(cursor_result);

        for (size_t i = 0; i < windows_.size(); ++i) {
            assert(index_section_0_ < windows_.size());
            if (i == index_section_0_) {
                std::cout << colors::grey << colors::on_white;
            } else {
                std::cout << colors::reset;
            }

            std::cout << windows_[i].title_ << " [" << windows_[i].class_name_ << "] (" \
                      << windows_[i].process_id_ << ")\n";
            std::cout << colors::reset;
        }

        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!GetConsoleScreenBufferInfo(console_handle, &info)) {
            constexpr std::string_view kActionDescription = "get the console screen buffer.";
            constexpr std::string_view kQualifiedName = "console.cc::fsb::Console::ShowMenu";
            constexpr std::string_view kExportedOperationName =
                "Kernel32.dll!GetConsoleScreenBufferInfo";
            constexpr int kReturnCode = 0;
            WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnCode);
            std::exit(FSB_INVALID_HANDLE);
        }

        int width = info.srWindow.Right \
            - info.srWindow.Left + 1;
        int height = info.srWindow.Bottom \
            - info.srWindow.Top + 1;
        int cursor_y = info.dwCursorPosition.Y \
            + info.srWindow.Top;

        int lines_needed = (height - 6) - cursor_y;

        for (int i = 0; i < lines_needed; ++i) {
            std::cout << "\n";
        }

        std::string row(width, '=');
        std::cout << row;
        std::cout << "Controls go here.";

        DispatchKeyPress(static_cast<char>(_getch()), &windows_[index_section_0_]);
    }


}

} // namespace fsb