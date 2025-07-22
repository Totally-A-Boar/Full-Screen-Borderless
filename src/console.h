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
    static std::string GetProcessFileName(uint32_t process_id);
    void ClearConsole();
    static int EnumWindowsCallback(HWND window_handle, LPARAM message_param);
    void RefreshWindows();
    void DispatchKeyPress(char key, ProcessData* process_data);

    bool clear_console_;
    int refresh_line_;
    bool menu_section_;
    int index_section_0_;
    int index_section_1_x_;
    int index_section_1_y_;
    std::vector<ProcessData> windows_;
};
}

#endif
