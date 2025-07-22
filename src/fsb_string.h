// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#ifndef FSB_STRING_H_
#define FSB_STRING_H_

#include "fsb_assert.h"

#include <utf8.h>
#include <string_view>

namespace fsb {
//! @brief Converts a UTF-16 string (std::wstring) to a UTF-8 string (std::string).
//!
//! This function takes an std::wstring_view (UTF-16), and using the utfcpp library, converts it to
//! an std::string (UTF-8) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
inline std::string Utf16ToUtf8(std::wstring_view input) {
    const auto* u16buf = reinterpret_cast<const char16_t*>(input.data());
    std::string output;
    static_cast<void>(utf8::utf16to8(u16buf, u16buf + input.size(),
        std::back_inserter(output)));
    FSB_ASSERT(!output.empty(), "Non-null conversion",
               "Converted strings should never return null");
    return output;
}

//! @brief Converts a UTF-8 string (std::string) to a UTF-16 string (std::wstring).
//!
//! This function takes an std::string_view (UTF-8), and using the utfcpp library, converts it to
//! an std::wstring (UTF-16) type.
//!
//! @param input The wide string input to be converted.
//! @returns Returns the UTF-8 output of the inputted UTF-16 string.
inline std::wstring Utf8ToUtf16(std::string_view input) {
    const auto* u8buf = reinterpret_cast<const uint8_t*>(input.data());
    std::wstring output;
    // Cast to void as return value isn't needed
    static_cast<void>(utf8::utf8to16(u8buf, u8buf + input.size(),
        std::back_inserter(output)));
    FSB_ASSERT(!output.empty(), "Non-null conversion",
               "Converted strings should never return null");
    return output;
}
} // namespace fsb

#endif // #ifndef FSB_STRING_H_