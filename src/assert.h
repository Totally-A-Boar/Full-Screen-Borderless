//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        assert.h
//
// Description: Contains custom assert macro and function for detailed error reporting and crash
//              dumping.
//
// Comments:    None
//
// Version:     1.0.0
//
// Classes:     None
//
// Functions:   fsb::FsbAssert(bool expr, std::string_view expr_str, std::string_view expr_name,
//                              std::string_view description, std::string_view file, int line,
//                              std::string_view function_name)
//
// Macros:      FSB_ASSERT(expr, name, description)
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================

#ifndef FSB_ASSERT_HPP_
#define FSB_ASSERT_HPP_

#include <iostream>
#include <string_view>

namespace fsb {
//! @brief Custom assert function to meet my needs for the project.
//!
//! This function is a custom-made assert function that can be used to display detailed assert
//! information upon an expression failing, while also generating a core dump.
//!
//! @param expr The expression that will be evaluated by the function. Main If the expression fails
//! evaluate to the desired result, the assertion fails, an error message is written to stderr,
//! and std::abort is called, generating a hard error and dumping the core.
//!
//! @param expr_str The string version of the expression passed. Handled by macro.
//!
//! @param expr_name The name of the expression passed. Example: "Math check" and the expression
//! could be "2 + 2 == 4."
//!
//! @param description A description of the expression passed. Example:
//! "Checks to be absolutely certain that 2+2 is equal to 4."
//!
//! @param file The name of the file that assert is being called from. Handled by macro.
//!
//! @param line The line of the file that assert is being called from. Handled by macro
//!
//! @param function_name The name of the function that assert is being called from. Handled by
//! macro.
//!
//! @warning This function should not be called directly, please use FSB_ASSERT.
inline void FsbAssert(bool expr, std::string_view expr_str, std::string_view expr_name,
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

//! @brief Custom assert function to meet my needs for the project.
//!
//! This function is a custom-made assert function that can be used to display detailed assert
//! information upon an expression failing, while also generating a core dump.
//!
//! @param expr The expression that will be evaluated by the function. Main If the expression fails
//! evaluate to the desired result, the assertion fails, an error message is written to stderr,
//! and std::abort is called, generating a hard error and dumping the core.
//!
//! @param expr_name The name of the expression passed. Example: "Math check" and the expression
//! could be "2 + 2 == 4."
//!
//! @param description A description of the expression passed. Example:
//! "Checks to be absolutely certain that 2+2 is equal to 4."
#define FSB_ASSERT(expr, name, description) \
    fsb::FsbAssert((expr), #expr, name, description, __FILE__, __LINE__, __func__)

#endif
