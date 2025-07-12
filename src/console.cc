//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        console.cc
//
// Description: Contains the implementation of the logic and functions for the TUI system
//
// Comments:    None
//
// Version:     1.0.0
//
// Classes:     fsb::Console
//
// Functions:
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#include "console.h"
#include "error.h"
#include "string.h" // Local string.h, not STL string.h
#include <fcntl.h>
#include <io.h>
#include <sstream>
#include <string>

namespace fsb {
Console::Console() {
    // Initialize the console
    // Hide the blinking cursor
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandle == INVALID_HANDLE_VALUE) {
        // Exit the application right after
        WIN32_ERROR("setup the console for correct I/O.", "fsb::Console::Console",
            "Kernel32.dll!GetStdHandle", -1);
        std::exit(kFsbConsoleInitFailure);
    }
    CONSOLE_CURSOR_INFO consoleCursorInfo;
    (void)GetConsoleCursorInfo(consoleHandle, &consoleCursorInfo);
    consoleCursorInfo.bVisible = false;
    (void)SetConsoleCursorInfo(consoleHandle, &consoleCursorInfo);

    // Active code page needs to be set to UTF-8 for proper Unicode output
    if (!SetConsoleOutputCP(CP_UTF8) || !SetConsoleCP(CP_UTF8)) {
        WIN32_ERROR("set the console output code page.", "fsb::Console::Console",
            "Kernel32.dll!SetConsoleOutputCP", 0);
        std::exit(kFsbConsoleInitFailure);
    }

    // Set the console mode to text
    // _setmode returns the previous translation mode of the console, which we do not need, so cast
    // to void.
    (void)_setmode(_fileno(stdout), _O_TEXT);
    (void)_setmode(_fileno(stderr), _O_TEXT);
    (void)_setmode(_fileno(stdin), _O_TEXT);

    // Set the title and cast to void because it shouldn't fail unless there are memory issues
    // which would probably crash the app or the OS entirely.
    (void)SetConsoleTitleW(L"Full Screen Borderless");
}

Console::~Console() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandle == INVALID_HANDLE_VALUE) {
        // Exit the application right after
        WIN32_ERROR("uninitialize the console.", "fsb::Console::~Console",
            "Kernel32.dll!GetStdHandle", -1);
        std::exit(kFsbConsoleUninitFailure);
    }

    CONSOLE_CURSOR_INFO cursorInfo;

    (void)GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.dwSize = 25;
    cursorInfo.bVisible = true;
    (void)SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

bool Console::GetWindowAttributes(HWND windowHandle, WindowAttributes* windowAttributes) {
    // Ensure the passed handle is valid
    if (windowHandle == nullptr || !IsWindow(windowHandle)) {
        return false;
    }

    // Check whether the window is visible
    bool isWindowVisible = IsWindowVisible(windowHandle);

    // Check whether the window is enabled
    bool isWindowEnabled = IsWindowEnabled(windowHandle);

    // Check the window state.
    WindowState windowState = WindowState::Normal;
    WINDOWPLACEMENT windowPlacement;
    windowPlacement.length = sizeof(windowPlacement);

    if (GetWindowPlacement(windowHandle, &windowPlacement)) {
        // showCmd is the property used to determine window state.
        // For example, nCmdShow in the WinMain entry point is the state to show the window in.
        // I used to think it meant whether to show the CMD window or not, however it means show
        // command not show command prompt.
        // It's only real use is this and ShowWindow which is the prefix in the macro SW means.
        // The values we need here are SW_SHOWMAXIMIZED, SW_SHOWMINIMIZED, SW_SHOWNORMAL and/or
        // SW_RESTORE (these two mean the same and have the same value).
        switch (windowPlacement.showCmd) {
            case SW_SHOWMAXIMIZED:
                windowState = WindowState::Maximized;
                break;
            case SW_SHOWMINIMIZED:
                windowState = WindowState::Minimized;
                break;
            case SW_SHOWNORMAL:
            case SW_RESTORE:
                windowState = WindowState::Normal;;
        }
    } else {
        return false;
    }

    windowAttributes->isEnabled = isWindowEnabled;
    windowAttributes->isVisible = isWindowVisible;
    windowAttributes->state = windowState;

    return true;
}

void Console::ClearConsole() {
    // Enable ANSI escape codes because Windows consoles don't interpret them by default
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandle == INVALID_HANDLE_VALUE) {
        // The application does not need to exit if this call fails, though we should log it
        // for debugging.
        WIN32_ERROR("clear the console.", "fsb::Console::ClearConsole",
            "Kernel32.dll!GetStdHandle", -1);
    }

    DWORD mode = 0;
    if (consoleHandle != INVALID_HANDLE_VALUE && GetConsoleMode(consoleHandle, &mode)) {
        // Only set the flag if it isn't set
        if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            SetConsoleMode(consoleHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    std::cout << u8"\033c[2J\033[1;1H";
}

int Console::EnumWindowsProcedure(HWND windowHandle, LPARAM messageParam) {
    // messageParam is unused, however, it cannot be omitted from the function signature as
    // EnumWindows expects this signature
    UNREFERENCED_PARAMETER(messageParam);

    if (windowHandle == nullptr || !IsWindow(windowHandle)) {
        return FSB_NULL_ARGUMENT; // The window is null so skip this iteration
    }

    // Save the display information
    DWORD processId = 0;
    // GetWindowThreadProcessId will return 0 on failure
    if (GetWindowThreadProcessId(windowHandle, &processId) == 0) {
        std::string_view actionDescription = "get the process ID for a window.";
        std::string_view qualifiedName = "console.cc::fsb::Console::EnumWindowsProcedure";
        std::string_view exportedOperationName = "User32.dll!GetWindowThreadProcessId";
        WIN32_ERROR(actionDescription, qualifiedName, exportedOperationName, 0);
    }

    wchar_t titleBuffer[256];
    // GetWindowTextW returns the length of the string it wrote. This function will always
    // truncate to the size of your buffer -1 for the null terminator (in this case 255).
    // On failure this function sets the last error to the issue that it ran in to, so you have
    // to be sure to check that on failure.
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
    if (GetWindowTextW(windowHandle, titleBuffer, std::size(titleBuffer)) == 0) {
        // The interface/messaging system to get the window title (WM_GETTEXT) was not found in the
        // window procedure for the inputted handle. We can safely ignore this error.
        if (GetLastError() != 0 && GetLastError() != ERROR_SEM_NOT_FOUND) {
            std::string_view actionDescription = "get the title of a window.";
            std::string_view qualifiedName = "console.cc::fsb::Console::EnumWindowsProcedure";
            std::string_view exportedOperationName = "User32.dll!GetWindowTextW";
            WIN32_ERROR(actionDescription, qualifiedName, exportedOperationName, GetLastError());

            return FSB_GENERIC_FAILURE;
        }
    }

    wchar_t classBuffer[256];
    // WNDCLASSEX only allows 256 characters so there should be no truncation issues should be here
    // This one also fails with return code 0, but, since every window has to be registered with an
    // actual window class name, it should always return. Access denied is the only ignorable error
    // in this case, however, I still log it.
    if (GetClassNameW(windowHandle, classBuffer, std::size(classBuffer)) == 0) {
        if (GetLastError() != 0) {
            std::string_view actionDescription = "get the class name of a window.";
            std::string_view qualifiedName = "console.cc::fsb::Console::EnumWindowsProcedure";
            std::string_view exportedOperationName = "User32.dll!GetClassNameW";
            WIN32_ERROR(actionDescription, qualifiedName, exportedOperationName,
                GetLastError());
        }
    }

    wchar_t fileBuffer[256];
    auto moduleHandle = reinterpret_cast<HMODULE>(GetWindowLongPtrW(windowHandle,
        GWLP_HINSTANCE));
    if (GetModuleFileNameW(moduleHandle, fileBuffer, std::size(fileBuffer)) == 0) {
        FSB_ASSERT(GetLastError() != ERROR_INSUFFICIENT_BUFFER, "Buffer is too small",
            "The buffer for GetModuleFileNameW is too small");
        if (GetLastError() != 0) {
            std::string_view actionDescription = "get the file name of a process.";
            std::string_view qualifiedName = "console.cc::fsb::Console::EnumWindowsProcedure";
            std::string_view exportedOperationName = "Kernel32.dll!GetModuleFileNameW";
            WIN32_ERROR(actionDescription, qualifiedName, exportedOperationName,
                GetLastError());
        }
    }

    WindowAttributes windowAttributes = {};
    if (!GetWindowAttributes(windowHandle, &windowAttributes)) {
        std::string_view actionDescription = "get the attributes for a window";
        std::string_view qualifiedName = "console.cc::fsb::Console::EnumWindowsProcedure";
        std::string_view exportedOperationName = "fsb.exe!GetWindowAttributes";
        WIN32_ERROR(actionDescription, qualifiedName, exportedOperationName,
            GetLastError());
    }

    // To-do: finish loop implementation and store variables in the class vector.

    return 0;
}

}