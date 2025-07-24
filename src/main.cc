// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#include "console.h"
#include "config.h"

int __stdcall wmain() {
    fsb::Config config = fsb::ParseConfig();
    fsb::Console console(config);

    console.ShowMenu();

    return 0;
}

