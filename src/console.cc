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
// Classes:     fsb::console
//
// Functions:
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#include "console.hh"
#include <io.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include "error.hh"
#include "string.hh"

namespace fsb {
Console::Console() {
    // Initialize the console
    // Hide the blinking cursor
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        // Exit the application right after
        WIN32_ERROR("setup the console for correct I/O.", "fsb::Console::Console",
            "Kernel32.dll!GetStdHandle", -1);
        std::exit(FSB_CONSOLE_INIT_FAILURE);
    }
    CONSOLE_CURSOR_INFO console_cursor_info;
    (void)GetConsoleCursorInfo(console_handle, &console_cursor_info);
    console_cursor_info.bVisible = false;
    (void)SetConsoleCursorInfo(console_handle, &console_cursor_info);

    // Active code page needs to be set to UTF-8 for proper Unicode output
    if (!SetConsoleOutputCP(CP_UTF8) || !SetConsoleCP(CP_UTF8)) {
        WIN32_ERROR("set the console output code page.", "fsb::Console::Console",
            "Kernel32.dll!SetConsoleOutputCP", 0);
        std::exit(FSB_CONSOLE_INIT_FAILURE);
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
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        // Exit the application right after
        WIN32_ERROR("uninitialize the console.", "fsb::Console::~Console",
            "Kernel32.dll!GetStdHandle", -1);
        std::exit(FSB_CONSOLE_UNINIT_FAILURE);
    }

    CONSOLE_CURSOR_INFO cursor_info;

    (void)GetConsoleCursorInfo(console_handle, &cursor_info);
    cursor_info.dwSize = 25;
    cursor_info.bVisible = true;
    (void)SetConsoleCursorInfo(console_handle, &cursor_info);
}

void Console::clear_console() {
    // Enable ANSI escape codes because Windows consoles don't interpret them by default
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {
        // The application does not need to exit if this call fails, though we should log it
        // for debugging.
        WIN32_ERROR("clear the console.", "fsb::Console::clear_console",
            "Kernel32.dll!GetStdHandle", -1);
    }

    DWORD mode = 0;
    if (console_handle != INVALID_HANDLE_VALUE && GetConsoleMode(console_handle, &mode)) {
        // Only set the flag if it isn't set
        if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            SetConsoleMode(console_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    std::cout << u8"\033c[2J\033[1;1H";
}



}