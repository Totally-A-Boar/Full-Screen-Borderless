//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        base_types.hpp
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
};

//! @brief Basic 2 integer vector for 2d sizes.
struct size_vec2 {
    uint32_t width;
    uint32_t height;
};
}

#endif