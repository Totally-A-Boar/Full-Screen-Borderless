//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        console.cpp
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

#include "console.hpp"
#include "string.hpp"
#include "error.hpp"
#include <string>
#include <sstream>

namespace fsb {
Console::Console() {
    // Initialize the console
    // Hide the blinking cursor
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_handle == INVALID_HANDLE_VALUE) {

    }
    CONSOLE_CURSOR_INFO console_cursor_info;
    (void)GetConsoleCursorInfo(console_handle, &console_cursor_info);
    console_cursor_info.bVisible = false;
    (void)SetConsoleCursorInfo(console_handle, &console_cursor_info);
}
}