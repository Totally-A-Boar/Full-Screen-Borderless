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

    void ShowMenu();
private:
    static bool GetWindowAttributes(HWND window_handle, WindowAttributes* window_attributes);
    static bool GetWindowMetrics(HWND window_handle, WindowMetrics* window_metrics);
    void ClearConsole();
    static int EnumWindowsCallback(HWND window_handle, LPARAM message_param);
    void RefreshWindows();
    bool SetWindowStyle(HWND window_handle, uint32_t style);
    std::vector<ProcessData> windows_;
};
}

#endif
