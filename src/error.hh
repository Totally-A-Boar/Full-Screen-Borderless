//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        error.hh
//
// Description: Contains the constants/macros used by the application for error code returns
//
// Comments:    This file doesn't contain very many error codes yet, but will expand with the
//              project.
//
// Version:     1.0.0
//
// Classes:     None
//
// Functions:   fsb::
//
// Macros:      FSB_NO_ERROR              (decimal: 0)
//              FSB_GENERIC_FAILURE       (decimal signed: 4209068033, unsigned: -858998263)
//              FSB_CONSOLE_INIT_FAILURE  (decimal signed: 4209068034, unsigned: -858998262)
//              FSB_NO_FOREGROUND_WINDOWS (decimal signed: 4209068035, unsigned: -858998261)
//              FSB_DEBUG_ASSERT_ERROR    (decimal signed: 4209068036, unsigned: -858998260)
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#ifndef FSB_ERROR_HPP_
#define FSB_ERROR_HPP_

#include <Windows.h>
#include <sstream>
#include <string>
#include "error.hh"
#include "string.hh"

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

namespace fsb {
inline void win32_error(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    wchar_t* buffer = nullptr;
    (void)FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, GetLastError(),
        0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

    std::wstring description = buffer ? buffer : L"Unknown error.";
    if (buffer) LocalFree(buffer);

    std::ostringstream oss;
    oss << u8"An error occurred while trying to " << action_description << "\r\n\r\n" \
           u8"Location: Line " << line << "fsb.exe (" << qualified_name << ")\r\n" \
           u8"Operation: " << exported_operation_name << "\r\n" \
           u8"Return value: "<< return_code << u8"\r\n" \
           u8"Error code: " << GetLastError() << u8"\r\n" \
           u8"Description: " << description.c_str();

    std::cerr << oss.str();
}

inline void failfast_win32(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    win32_error(action_description, line, qualified_name, exported_operation_name, return_code);
    std::abort();
}

inline void stl_error(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    std::ostringstream oss;
    oss << u8"An error occurred while trying to " << action_description << "\r\n\r\n" \
           u8"Location: Line " << line << "fsb.exe (" << qualified_name << ")\r\n" \
           u8"Operation: " << exported_operation_name << "\r\n" \
           u8"Return value: "<< return_code << u8"\r\n" \
           u8"Error code: " << errno << u8"\r\n";

    std::cerr << oss.str();
}

inline void failfast_stl(std::string_view action_description, int line,
    std::string_view qualified_name, std::string_view exported_operation_name,
    int32_t return_code) {
    stl_error(action_description, line, qualified_name, exported_operation_name, return_code);
    std::abort();
}
}

#define WIN32_ERROR(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::win32_error(action_description, __LINE__, qualified_name, exported_operation_name, \
                     return_code)
#define WIN32_FAILFAST(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::failfast_win32(action_description, __LINE__, qualified_name, exported_operation_name, \
                        return_code)
#define STL_ERROR(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::stl_error(action_description __LINE__, qualified_name, exported_operation_name, \
                   return_code)
#define STL_FAILFAST(action_description, qualified_name, exported_operation_name, return_code) \
    fsb::failfast_stl(action_description __LINE__, qualified_name, exported_operation_name, \
                      return_code)

#endif