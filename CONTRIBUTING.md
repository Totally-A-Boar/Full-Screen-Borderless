# Contributing

## Style guide
This project uses a custom clang-format to meet my preferences for clean code. It also has a clang-tidy to complement it. The clang-tidy may not be applicable to everyone as it is reliant on the IDE and linter, however, most IDEs have plugins for clang-tidy support. The clang-format can be summarized as 'a slightly modifed Chromium base.' This project follows a custom-made style guide that is a sort of mishmash of other style guides.

As a base set of rules:
- 100 column line length max
- LF (Unix-style) line endings (Git will should automatically convert everything to LF)
- 4-space soft tabs, no tab key (0x09 in the file's bytes)
- `snake_case` naming/casing conventions
- Variables should always have descriptive names unless it is a trivial value/variable
- Functions should always be cast to void if the return value isn't used (not applicable if the function returns void)
- Comments and strings should always be properly punctuated (punctiation, capitilization, hyphens, etc)
- Sort include by order of importance (for example `Windows.h` comes before `WinSock2.h`)

Files should also have a header that resembles the following at the top. This header states the project name, file name, a description of the file, any comments from authors, file version (after being in a release build), classes/structs declared or exported in the file, functions declared, exported, or implemented in the file, any macros declared or exported in the file, and, a copyright/legal notice.
```
//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        main.cpp
//
// Description: Main entry point for FSB. Contains most logic for program.
//
// Comments:    None
//
// Version:     2.0.0
//
// Classes:     fsb::process_window
//
// Functions:   fsb::init_console()
//              fsb::uninit_console()
//              fsb::EnumWindowsProc(HWND window_handle, LPARAM message_param)
//              fsb::fullscreen_window(HWND window_handle)
//              fsb::clear_console()
//              fsb::show_console_menu()
//              wmain()
//
// Macros:      None
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================
```
