//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        error.h
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
// Functions:   None
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

#endif