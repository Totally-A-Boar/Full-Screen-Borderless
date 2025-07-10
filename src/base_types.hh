//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        base_types.hh
//
// Description: Contains base types used by the program
//
// Comments:    None
//
// Version:     1.0.0
//
// Classes:     fsb::process_data
//              fsb::size_vec2
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
struct size_vec2 {
    uint32_t x;
    uint32_t y;
};

struct window_metrics {
    size_vec2 position;

    size_vec2 size;

    std::string font_name;

    uint32_t font_size;

    //! @note DWORD type stored as uint32_t. Should always cast to DWORD when using or cast to
    //! uint32_t when storing.
    uint32_t style;

    uint32_t ex_style;
};

enum class window_state {
    NORMAL,
    MAXIMIZED,
    MINIMIZED
};

struct window_attributes {
    bool is_visible;

    bool is_enabled;

    window_state state;
};

//! @brief Holds information about a process and its associated window.
//!
//! This structure is the main structure used within the application to manage the window's full
//! screen properties, and the window's title and process ID for display to the user
struct process_data {
    //! Handle to the window. This is the actual property used by the application to apply the
    //! full-screen property
    HWND window_handle;

    //! The process ID associated with the window/application
    uint32_t process_id;

    //! Title of the window to display in the interface
    std::string title;

    std::string class_name;

    std::string file_name;

    window_attributes attributes;

    window_metrics metrics;
};



}

#endif