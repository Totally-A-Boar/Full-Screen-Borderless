// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#ifndef FSB_CONFIG_H_
#define FSB_CONFIG_H_

#include <string>

namespace fsb {
struct Config {
    bool hide_hidden_windows_;
    bool hide_blank_title_windows_;
};

std::string GetUserDirectory();
Config ParseConfig();

} // namespace fsb

#endif // #ifndef FSB_CONFIG_H_