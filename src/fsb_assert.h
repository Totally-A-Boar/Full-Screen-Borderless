// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

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
    std::string_view description, std::string_view file, int32_t line,
    std::string_view function_name) {
    if (!expr) {
        std::cerr << "Assertion '" << expr_name << "' failed!\n" \
                  << "Expression: " << expr_str << "\n" \
                  << "Description: " << description << "\n" \
                  << "Location: line " << line << ", " << file << ":" << function_name \
                  << "\n";
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
