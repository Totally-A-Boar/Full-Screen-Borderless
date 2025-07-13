// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#ifndef FSB_ERROR_HPP_
#define FSB_ERROR_HPP_

#include "error.h"
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

    std::wstring description = buffer ? buffer : L"Unknown error.";
    if (buffer) LocalFree(buffer);

    std::ostringstream oss;
    oss << "An error occurred while trying to " << actionDescription << "\r\n\r\n" \
           "Location: Line " << line << "fsb.exe (" << qualifiedName << ")\r\n" \
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
           "Location: Line " << line << "fsb.exe (" << qualified_name << ")\r\n" \
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
}

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

#endif