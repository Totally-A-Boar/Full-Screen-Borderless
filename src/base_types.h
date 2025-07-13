//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        base_types.h
//
// Description: Contains base types used by the program
//
// Comments:    None
//
// Version:     1.0.0
//
// Classes:     fsb::ProcessData
//              fsb::SizeVec2
//
// Functions:   None
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#ifndef FSB_BASETYPES_H_
#define FSB_BASETYPES_H_

#include <cstdint>
#include <string>
#include <Windows.h>

namespace fsb {

//! @brief Basic 2 integer vector for 2d sizes.
struct SizeVec2 {
    int32_t x;
    int32_t y;
};

struct WindowMetrics {
    SizeVec2 position;
    SizeVec2 size;
    std::string fontName;
    uint32_t fontSize;
    //! @note DWORD type stored as uint32_t. Should always cast to DWORD when using or cast to
    //! uint32_t when storing.
    uint32_t style;
    uint32_t exStyle;
};

enum class WindowState {
    Normal,
    Maximized,
    Minimized
};

struct WindowAttributes {
    bool isVisible;
    bool isEnabled;
    WindowState state;
};

//! @brief Holds information about a process and its associated window.
//!
//! This structure is the main structure used within the application to manage the window's full
//! screen properties, and the window's title and process ID for display to the user.
struct ProcessData {
    //! Handle to the window. This is the actual property used by the application to apply the
    //! full-screen property.
    HWND windowHandle;
    //! The process ID associated with the window/application.
    uint32_t processId;
    //! Title of the window to display in the interface.
    std::string title;
    std::string className;
    std::string fileName;
    WindowAttributes attributes;
    WindowMetrics metrics;
};

}  // namespace fsb

#endif  // FSB_BASETYPES_H_