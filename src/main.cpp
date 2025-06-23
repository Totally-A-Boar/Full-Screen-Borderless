//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        main.cpp
//
// Description: Main entry point for FSB. Contains most logic for program.
//
// Comments:    None
//
// Version:     2.0.0
//
// Classes:     fsb::process_window
//
// Functions:   fsb::utf16_to_utf8(std::wstring_view)
//              fsb::utf8_to_utf16(std::string_view)
//              fsb::init_console()
//              fsb::uninit_console()
//              fsb::EnumWindowsProc(HWND window_handle, LPARAM message_param)
//              fsb::fullscreen_window(HWND window_handle)
//              fsb::clear_console()
//              fsb::show_console_menu()
//              wmain()
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#include "error.h"
#include <Windows.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <utfcpp/source/utf8.h>
#include <colors/include/colors/colors.hpp>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "assert.hpp"

namespace fsb {
//! @brief Holds information about a process and its associated window.
//!
//! This structure is the main structure used within the application to manage the window's full
//! screen properties, and the window's title and process ID for display to the user
struct process_window {
    //! The process ID associated with the window/application
    uint32_t process_id;

    //! Handle to the window. This is the actual property used by the application to apply the
    //! full-screen property
    HWND window_handle;

    //! Title of the window to display in the interface
    std::string title;
};

// Global variable(s)
//! Vector of all foreground windows. Contains display data and window handle to full screen it.
std::vector<process_window> windows;

// Functions
//! @brief Converts a UTF-16 string (std::wstring) to a UTF-8 string (std::string).
//!
//! This function takes an std::wstring_view (UTF-16), and using the utfcpp library, converts it to
//! an std::string (UTF-8) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
std::string utf16_to_utf8(std::wstring_view input) {
    const auto* u16buf = reinterpret_cast<const char16_t*>(input.data());
    std::string output;
    // Cast to void as return value isn't needed
    (void)utf8::utf16to8(u16buf, u16buf + input.size(),
        std::back_inserter(output));
    FSB_ASSERT(!output.empty(), "Non-null string", "Converted strings should never return null");
    return output;
}

//! @brief Converts a UTF-8 string (std::string) to a UTF-16 string (std::wstring).
//!
//! This function takes an std::string_view (UTF-8), and using the utfcpp library, converts it to
//! an std::wstring (UTF-16) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
std::wstring utf8_to_utf16(std::string_view input) {
    const auto* u8buf = reinterpret_cast<const uint8_t*>(input.data());
    std::wstring output;
    // Cast to void as return value isn't needed
    (void)utf8::utf8to16(u8buf, u8buf + input.size(), std::back_inserter(output));
    FSB_ASSERT(!output.empty(), "Non-null string", "Converted strings should never return null");
    return output;
}

//! @brief Initializes the console for proper UTF-8 I/O.
//!
//! This function sets up the standard console streams for UTF-8 output to ensure proper handling
//! of Unicode characters from cout (UTF-8) in the console output streams.
//!
//! It also sets the console to the proper code page and sets the title for cleanliness and to look
//! as proper as possible.
//!
//! @note this function casts the calls within it because it is non-critical and shouldn't fail
//! unless there are larger problems than just the one call failing
//! (e.g. the system is out of memory).
void init_console() {
    // Casting to void because if these fail, it won't break the application
    // Hide the cursor
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor_info;

    (void)GetConsoleCursorInfo(console_handle, &cursor_info);
    cursor_info.dwSize = 1;
    cursor_info.bVisible = FALSE;
    (void)SetConsoleCursorInfo(console_handle, &cursor_info);

    // Active code page needs to be set to UTF-8 for proper Unicode output
    if (!SetConsoleOutputCP(CP_UTF8) || !SetConsoleCP(CP_UTF8)) {
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, GetLastError(),
            0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

        std::string description = utf16_to_utf8(buffer ? buffer : L"Unknown error.");
        if (buffer) LocalFree(buffer);

        std::ostringstream oss;
        oss << u8"An error occurred while trying to set the code page for the console.\r\n\r\n" \
               u8"Location: Line 100, fsb.exe (Main.cpp::init_console)\r\n" \
               u8"Operation: Kernel32.dll!SetConsoleOutputCP\r\n" \
               u8"Return value: false" << u8"\r\n" \
               u8"Error code: " << GetLastError() << u8"\r\n" \
               u8"Description: " << description.c_str();

        std::cerr << oss.str();
        exit(FSB_CONSOLE_INIT_FAILURE);
    }

    // Set the console mode to text
    (void)_setmode(_fileno(stdout), _O_TEXT);
    (void)_setmode(_fileno(stderr), _O_TEXT);
    (void)_setmode(_fileno(stdin), _O_TEXT);

    // Set the title and cast to void because it shouldn't fail unless there are memory issues
    // which would probably crash the app or the OS entirely.
    (void)SetConsoleTitleW(L"Full Screen Borderless");
}

//! @brief Resets the cursor for the console.
//!
//! This function restores the visibility of the cursor for the console.
void uninit_console() {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor_info;

    GetConsoleCursorInfo(console_handle, &cursor_info);
    cursor_info.dwSize = 25;
    cursor_info.bVisible = true;
    SetConsoleCursorInfo(console_handle, &cursor_info);
}

//! @brief Callback function used with EnumWindows to collect all windows currently running on the
//! system
//!
//! This function is called by the Windows API for and contains custom logic to loop through all
//! windows and add it to the vector for the process_window struct if it is visible.
//!
//! @param window_handle Handle to the current window that is being looped through. Is used to
//! obtain the title, process ID, and saves the handle itself
//! @param message_param Unused parameter; required by the API.
int EnumWindowsProc(HWND window_handle, LPARAM message_param) {
    // Function signature has to match, but I don't use the LPARAM, so use UNREFERENCED_PARAMETER
    // to avoid the compiler complaining it is unreferenced
    UNREFERENCED_PARAMETER(message_param);

    if (window_handle == nullptr || !IsWindow(window_handle)) {
        return 1; // Continue enumeration, but skip this iteration
    }

    // Saved for display information
    DWORD process_id = 0;
    if (GetWindowThreadProcessId(window_handle, &process_id) == 0) {
        wchar_t* buffer = nullptr; // It allocates the buffer so make it a pointer
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, GetLastError(),
            0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

        std::string description = utf16_to_utf8(buffer ? buffer : L"Unknown error.");
        if (buffer) LocalFree(buffer);

        std::ostringstream oss;
        oss << u8"An error occurred while trying to get the process ID for a window.\r\n\r\n" \
               u8"Location: Line 142, fsb.exe (Main.cpp::EnumWindowsProc)\r\n" \
               u8"Operation: User32.dll!GetWindowThreadProcessId\r\n" \
               u8"Return value: 0x0" << u8"\r\n" \
               u8"Error code: " << GetLastError() << u8"\r\n" \
               u8"Description: " << description.c_str();

        std::cerr << oss.str();
        return 1;
    }

    // Ditto
    wchar_t buffer[256];
    if (GetWindowTextW(window_handle, buffer, std::size(buffer)) == 0) {
        if (GetLastError() != 0 || GetLastError() == ERROR_SEM_NOT_FOUND) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::string description = utf16_to_utf8(err_buffer ? err_buffer : L"Unknown error.");
            if (err_buffer) LocalFree(err_buffer);

            std::ostringstream oss;
            oss << u8"An error occurred while trying to get the title for a window.\r\n\r\n" \
                   u8"Location: Line 128, fsb.exe (Main.cpp::EnumWindowsProc)\r\n" \
                   u8"Operation: User32.dll!GetWindowTextW\r\n" \
                   u8"Return value: 0x0" << u8"\r\n" \
                   u8"Error code: 0x" << std::hex << GetLastError() << u8"\r\n" \
                   u8"Description: " << description;

            std::cerr << oss.str();
            return 1;
        }
    }

    // Put the buffer into a proper string and check if the window is a foreground window
    if (std::string title = utf16_to_utf8(buffer);
        IsWindowVisible(window_handle) && !title.empty()) {
        // If so, add it to the vector
        windows.push_back({process_id, window_handle, title});
    }

    return 1;  // returning true
}

//! @brief Sets a window's properties to be in full-screen mode.
//!
//! This function will modify a given window to remove borders and resize it to cover the bounds of
//! the screen.
//!
//! @param window_handle Handle to the window that is to be modified.
void fullscreen_window(HWND window_handle) {
    // Make sure the window is visible
    SetWindowLongW(window_handle, GWL_STYLE, WS_VISIBLE);
    // Now set the window's properties to cover the full screen
    SetWindowPos(window_handle, HWND_TOP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED);
}

//! @brief Clears the console screen using ANSI escape codes.
//!
//! This function enables virtual terminal processing (ANSI escape codes) on Windows consoles
//! and then clears the screen and moves the cursor to the top-left corner using ANSI codes.
//! On platforms that already support ANSI codes (such as Linux or macOS terminals), no extra
//! setup is performed. This function avoids using the Windows Console API for clearing.
//!
//! @note On Windows, this function enables ANSI escape code support if it is not already enabled.
void clear_console() {
    // Enable ANSI escape codes because Windows consoles don't interpret them by default
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (console_handle != INVALID_HANDLE_VALUE && GetConsoleMode(console_handle, &mode)) {
        // Only set the flag if it isn't set
        if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            SetConsoleMode(console_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    std::cout << u8"\033c[2J\033[1;1H";
}

void show_console_menu() {
    int selected_x = 0;

    while (true) {
        clear_console();
        std::cout << u8"Select a process to fullscreen:\r\n\r\n";

        for (size_t i = 0; i < windows.size(); ++i) {
            std::cout << u8"   ";
            if (i == selected_x) {
                std::cout << colors::grey << colors::on_white;
            } else {
                std::cout << colors::reset;
            }

            std::cout << windows[i].title << u8" (Process Id: " << windows[i].process_id \
                      << u8")\r\n";
            std::cout << colors::reset;
        }

        // Check how many = signs to fill a row on the screen
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console_handle == INVALID_HANDLE_VALUE) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::string description = utf16_to_utf8(err_buffer ? err_buffer : L"Unknown error.");
            if (err_buffer) LocalFree(err_buffer);

            std::ostringstream oss;
            oss << u8"An error occurred while trying to get the handle to the console.\r\n\r\n" \
                   u8"Location: Line 228, fsb.exe (Main.cpp::show_console_menu)\r\n" \
                   u8"Operation: Kernel32.dll!GetStdHandle\r\n" \
                   u8"Error code: 0x" << std::hex << GetLastError() << u8"\r\n" \
                   u8"Description: " << description.c_str();

            std::cerr << oss.str();
            exit(FSB_GENERIC_FAILURE);
        }

        CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_info;
        if (!GetConsoleScreenBufferInfo(console_handle, &console_screen_buffer_info)) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::string description = utf16_to_utf8(err_buffer ? err_buffer : L"Unknown error.");
            if (err_buffer) LocalFree(err_buffer);

            std::ostringstream oss;
            oss << u8"An error occurred while trying to get the console buffer info.\r\n\r\n" \
                   u8"Location: Line 252, fsb.exe (Main.cpp::show_console_menu)\r\n" \
                   u8"Operation: Kernel32.dll!GetConsoleScreenBufferInfo\r\n" \
                   u8"Return value: false" << u8"\r\n" \
                   u8"Error code: 0x" << std::hex << GetLastError() << u8"\r\n" \
                   u8"Description: " << description.c_str();

            std::cerr << oss.str();
        }

        int width = console_screen_buffer_info.srWindow.Right \
            - console_screen_buffer_info.srWindow.Left + 1;
        int height = console_screen_buffer_info.srWindow.Bottom \
            - console_screen_buffer_info.srWindow.Top + 1;
        int cursor_y = console_screen_buffer_info.dwCursorPosition.Y \
            + console_screen_buffer_info.srWindow.Top;

        int lines_to_move = (height - 2) - cursor_y;

        for (int i = 0; i < lines_to_move; ++i) {
            std::cout << u8"\r\n";
        }

        std::string row(width, '=');

        std::cout << row << u8"\r\n";
        std::cout << u8"[" << colors::blue << u8"Q" << colors::reset << u8"] ";
        std::cout << u8"Quit ";
        std::cout << u8"[" << colors::blue << u8"R" << colors::reset << u8"] ";
        std::cout << u8"Reset ";
        std::cout << u8"[" << colors::blue << u8"↵" << colors::reset << u8"] ";
        std::cout << u8"Apply ";

        int key = _getch();
        if (key == 0x1B || key == 'Q' || key == 'q') {
            exit(0);
        }
        // Enter key
        if (key == '\r') {
            fullscreen_window(windows[selected_x].window_handle);
            exit(0);
        }
        // R key
        if (key == 'R' || key == 'r') {
            windows.clear();
            EnumWindows(EnumWindowsProc, 0);
        }
        // Up arrow
        if (key == 72) {
            if (selected_x > 0) selected_x--;
            else selected_x = static_cast<int>(windows.size()) - 1;
        }
        // Down arrow
        if (key == 80) {
            if (selected_x < static_cast<int>(windows.size()) - 1) selected_x++;
            else selected_x = 0;
        }
    }
}

} // namespace fsb

int __stdcall wmain() {
    // Set up the console for proper I/O
    fsb::init_console();

    // My EnumWindowsProc never returns false.
    (void)EnumWindows(fsb::EnumWindowsProc, 0);
    if (fsb::windows.empty()) {
        std::ostringstream oss;
        oss << u8"You have launched fsb with no foreground windows! " \
               u8"Please relaunch with at least one foreground window.";
        std::cerr << oss.str();
        return FSB_NO_FOREGROUND_WINDOWS;
    }

    fsb::show_console_menu();

    fsb::uninit_console();
    return 0;
}

