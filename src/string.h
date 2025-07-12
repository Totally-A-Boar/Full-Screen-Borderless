//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        string.h
//
// Description: Contains helper functions for converting UTF-16 to UTF-8 and vice versa
//
// Comments:    None
//
// Version:     1.0.0
//
// Classes:     None
//
// Functions:   fsb::utf16_to_utf8(std::wstring_view)
//              fsb::utf8_to_utf16(std::string_view)
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#ifndef FSB_STRING_HPP_
#define FSB_STRING_HPP_

#include "assert.h"
#include <utfcpp/source/utf8.h>
#include <string_view>

namespace fsb {
//! @brief Converts a UTF-16 string (std::wstring) to a UTF-8 string (std::string).
//!
//! This function takes an std::wstring_view (UTF-16), and using the utfcpp library, converts it to
//! an std::string (UTF-8) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
inline std::string utf16_to_utf8(std::wstring_view input) {
    const auto* u16buf = reinterpret_cast<const char16_t*>(input.data());
    std::string output;
    // Cast to void as return value isn't needed
    (void)utf8::utf16to8(u16buf, u16buf + input.size(),
        std::back_inserter(output));
    FSB_ASSERT(!output.empty(), "Non-null string", "Converted strings should never return null");
    return output;
}

//! @brief Converts a UTF-8 string (std::string) to a UTF-16 string (std::wstring).
//!
//! This function takes an std::string_view (UTF-8), and using the utfcpp library, converts it to
//! an std::wstring (UTF-16) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
inline std::wstring utf8_to_utf16(std::string_view input) {
    const auto* u8buf = reinterpret_cast<const uint8_t*>(input.data());
    std::wstring output;
    // Cast to void as return value isn't needed
    (void)utf8::utf8to16(u8buf, u8buf + input.size(), std::back_inserter(output));
    FSB_ASSERT(!output.empty(), "Non-null string", "Converted strings should never return null");
    return output;
}
}

#endif