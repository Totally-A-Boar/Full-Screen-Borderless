/*
*+=======================================================================+*
 * FSB : Full screen borderless                                            *
 *+=======================================================================+*
 * Copyright 2025 Jamie Howell. All rights reserved.                       *
 * Licensed under The MIT License -- see LICENSE in root for more details. *
 *+=======================================================================+*
 */

#ifndef FSB_ASSERT_HPP_
#define FSB_ASSERT_HPP_

#include <iostream>
#include <string_view>

namespace fsb {
//! @brief Custom assert function to meet my needs for the project.
//!
//! This function is a custom-made assert function that can be used to display detailed assert
//! information upon an expression failing.
//!
//! @param expr The expression that will be evaluated by the function. Main If the expression fails
//! evauluate to the desired result, the assertion fails, an error message is written to stderr,
//! std::abort is called, and the function exits with FSB_DEBUG_ASSERT_FAILURE (0xFB000006)
//!
inline void fsb_assert(bool expr, std::string_view expr_str, std::string_view expr_name,
    std::string_view description, std::string_view file, int line, std::string_view function_name) {
    if (!expr) {
        std::cerr << u8"Assertion '" << expr_name << "' failed!\n" \
                  << u8"Expression: " << expr_str << "\n" \
                  << u8"Description: " << description << "\n" \
                  << u8"Location: line " << line << u8", " << file << u8":" << function_name \
                  << u8"\n";
        std::abort();
    }
}
}

#define FSB_ASSERT(expr, name, desc) \
    fsb::fsb_assert((expr), #expr, name, desc, __FILE__, __LINE__, __func__)

#endif
