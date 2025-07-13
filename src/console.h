// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

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
    bool GetWindowMetrics(HWND windowHandle, WindowMetrics* windowMetrics);
    void ClearConsole();
    int EnumWindowsProcedure(HWND windowHandle, LPARAM messageParam);
    std::vector<ProcessData> windows_;
};
}

#endif
