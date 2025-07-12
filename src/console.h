//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        console.h
//
// Description: Contains the logic and functions for the TUI system
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

#ifndef FSB_CONSOLE_HPP_
#define FSB_CONSOLE_HPP_

#include "base_types.h"
#include <Windows.h>
#include <conio.h>
#include <string_view>
#include <vector>

namespace fsb {
class Console {
public:
    Console();
    ~Console();
private:
    bool GetWindowAttributes(HWND windowHandle, WindowAttributes* windowAttributes);
    void ClearConsole();
    int EnumWindowsProcedure(HWND windowHandle, LPARAM messageParam);
    std::vector<ProcessData> windows_;
};
}

#endif
