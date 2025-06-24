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

It is generally a good practice to run `format.sh` in root before pushing, however, I try to run it every commit.

Files should also have a header that resembles the following at the top. This header states the project name, file name, a description of the file, any comments from authors, file version (after being in a release build), classes/structs declared or exported in the file, functions declared, exported, or implemented in the file, any macros declared or exported in the file, and, a copyright/legal notice.
```
//+=================================================================================================
// Project:     fsb : Full Screen Borderless
//
// File:        my_file.ext
//
// Description: A summary of the file. Try to keep this 1-3 lines.
//
// Comments:    Any comments about the file by authors
//
// Version:     Version number goes here (SemVer format: X.X.X)
//
// Classes:     Classes/structs go here
//
// Functions:   Functions declared or exported go here
//
// Macros:      Macros declared or exported go here
//
// Copyright © 2025 Jamie Howell. All rights reserved
// Licensed under The MIT License. See LICENSE file in project root for full license, or, go to
// https://opensource.org/license/mit
//+=================================================================================================
```
This file header comes from the old Win32 Coding Style Conventions (https://learn.microsoft.com/en-us/windows/win32/stg/coding-style-conventions) which I customized and modified to make more modern and informative and turned it into what you see above.

Please make sure your pull requests follow these guidelines and run `format.sh` before pushing your request. Code that doesn't meet this standard or doesn't follow these guidelines will not be accepted.

## Tools needed
To write or compile code for this project, the following tools are needed:
### Compiler
See `Compiling.md`

### clang-format
clang-format is used for every source file in this project. Make sure to run it from root to use the project's clang-format settings. If you do not have it installed, run `format.sh` and instructions will be given to install it. It is recommended to use a Linux terminal for Windows (WSL or MSYS2 are my favorites). There are different commands for each package manager though, so if your terminal of choice isn't listed, please add it in a pull request.

### IDE
It is recommended to use an actual IDE, not just a text editor for those of you still writing code in Notepad++. I recommend CLion as it has strong integration with the build system and formatting system of this project. It is also now free for non-commerical (personal) use. It is my editor of choice and I highly recommend it to anyone who is looking for a different IDE. However, if you do not use this IDE, there are plugins available for clang-format and clang-tidy integration in other IDEs such as, VS Code, Visual Studio, and Vim.

#
With that, you are now suited up and ready to contribute code to this project. I hope to some day make it a full WinExp console clone. Thank you for reading!