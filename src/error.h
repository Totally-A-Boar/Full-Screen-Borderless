// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#ifndef FSB_ERROR_H_
#define FSB_ERROR_H_

#include "base_types.h"
#include "fsb_string.h"

#include <Windows.h>
#include <sstream>
#include <string>

// No error
#define FSB_NO_ERROR              0x00000000

// Generic failure fallback
#define FSB_GENERIC_FAILURE       0xFB000001

// Error initializing the console
#define FSB_CONSOLE_INIT_FAILURE  0xFB000002

// FSB was launched with no foreground windows
#define FSB_NO_FOREGROUND_WINDOWS 0xFB000003

// A debug assert call did not evaluate to the correct result
#define FSB_DEBUG_ASSERT_ERROR    0xFB000004

// A call in the console (Console::~Console) destructor failed
#define FSB_CONSOLE_UNINIT_FAILURE 0xFB000005

// A null argument was passed to a function expecting a non-null value
#define FSB_NULL_ARGUMENT          0xFB000006

// A handle or a call to get Win32-related failed
#define FSB_INVALID_HANDLE         0xFB000007

namespace fsb {
inline void Win32Error(std::string_view actionDescription, int line,
    std::string_view qualifiedName, std::string_view exportedOperationName,
    int32_t returnCode) {
    wchar_t* buffer = nullptr;
    (void)FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, GetLastError(),
        0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

    std::wstring wdescription = buffer ? buffer : L"Unknown error.";
    if (buffer) LocalFree(buffer);

    std::string description = Utf16ToUtf8(wdescription);

    std::ostringstream oss;
    oss << "An error occurred while trying to " << actionDescription << "\r\n\r\n" \
           "Location: Line " << line << ", fsb.exe (" << qualifiedName << ")\r\n" \
           "Operation: " << exportedOperationName << "\r\n" \
           "Return value: "<< returnCode << "\r\n" \
           "Error code: " << GetLastError() << "\r\n" \
           "Description: " << description.c_str();

    std::cerr << oss.str();
}

inline void FailfastWin32(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    Win32Error(action_description, line, qualified_name, exported_operation_name, return_code);
    std::abort();
}

inline void StlError(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    std::ostringstream oss;
    oss << "An error occurred while trying to " << action_description << "\r\n\r\n" \
           "Location: Line " << line << ", fsb.exe (" << qualified_name << ")\r\n" \
           "Operation: " << exported_operation_name << "\r\n" \
           "Return value: "<< return_code << "\r\n" \
           "Error code: " << errno << "\r\n";

    std::cerr << oss.str();
}

inline void FailfastStl(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    StlError(action_description, line, qualified_name, exported_operation_name, return_code);
    std::abort();
}

inline void DumpProcessData(const ProcessData& process_data) {
    std::cout << "Debug process data dump\n";
    std::cout << "Title: " << process_data.title_ << "\n";
    std::cout << "Class Name: " << process_data.class_name_ << "\n";
    std::cout << "File Name: " << process_data.file_name_ << "\n";
    std::cout << "Process ID: " << process_data.process_id_ << "\n";
    std::cout << "Window Handle: " << process_data.window_handle_ << "\n";

    std::cout << "Window Attributes:\n";
    std::cout << "  Is Visible: " << (process_data.attributes_.is_visible_ ? "Yes" : "No") << "\n";
    std::cout << "  Is Enabled: " << (process_data.attributes_.is_enabled_ ? "Yes" : "No") << "\n";
    std::cout << "  State: ";
    switch (process_data.attributes_.state_) {
        case WindowState::Normal: std::cout << "Normal"; break;
        case WindowState::Minimized: std::cout << "Minimized"; break;
        case WindowState::Maximized: std::cout << "Maximized"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << "\n";

    std::cout << "Window Metrics:\n";
    std::cout << "  Position: (" << process_data.metrics_.position_.x
              << ", " << process_data.metrics_.position_.y << ")\n";
    std::cout << "  Size: (" << process_data.metrics_.size_.x
              << ", " << process_data.metrics_.size_.y << ")\n";
    std::cout << "  Font Name: " << process_data.metrics_.font_name_ << "\n";
    std::cout << "  Font Size: " << process_data.metrics_.font_size_ << "\n";
    std::cout << "  Style: 0x" << std::hex << process_data.metrics_.style_ << std::dec << "\n";
    std::cout << "  ExStyle: 0x" << std::hex << process_data.metrics_.ex_style_ << std::dec << "\n";
    std::cout << "\n";
}
} // namespace fsb

#define WIN32_ERROR(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::Win32Error(action_description, __LINE__, qualified_name, exported_operation_name, \
                     return_code)
#define WIN32_FAILFAST(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::FailfastWin32(action_description, __LINE__, qualified_name, exported_operation_name, \
                        return_code)
#define STL_ERROR(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::StlError(action_description __LINE__, qualified_name, exported_operation_name, \
                   return_code)
#define STL_FAILFAST(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::FailfastStl(action_description __LINE__, qualified_name, exported_operation_name, \
                      return_code)

#endif // #ifndef FSB_ERROR_H_