// Copyright 2025 Jamie Howell
// Use of this source code is governed by an MIT license that can be
// found in the LICENSE file.

#include "config.h"

#include "fsb_string.h"
#include "error.h"

#include <ShlObj.h>
#include <Windows.h>
#include <cassert>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string_view>

#include "base_types.h"

std::string fsb::GetUserDirectory() {
    wchar_t* buffer = nullptr;
    HRESULT result = SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &buffer);
    if (FAILED(result)) {
        constexpr char kActionDescription[] = "get the user profile path.";
        constexpr char kQualifiedName[] = "config.cc::fsb::GetUserDirectory";
        constexpr char kExportedOperationName[] = "Shell32.dll!SHGetKnownFolderPath";
        const int kReturnValue = result;
        WIN32_ERROR(kActionDescription, kQualifiedName, kExportedOperationName, kReturnValue);

        return "$ERROR";
    }
    assert(SUCCEEDED(result) && buffer != nullptr);

    std::string final = Utf16ToUtf8(buffer);
    CoTaskMemFree(buffer);

    return final;
}

fsb::Config fsb::ParseConfig() {
    Config result = {true, true};

    std::string user_path = fsb::GetUserDirectory();
    if (user_path == "$ERROR") {
        std::exit(FSB_GENERIC_FAILURE);
    }

    std::string file_path = user_path + "\\.fsb";

    if (!std::filesystem::exists(file_path)) {
        result.hide_hidden_windows_ = true;
        result.hide_blank_title_windows_ = true;
        return result;
    }

    std::ifstream config_file(file_path);
    if (!config_file.is_open()) {
        result.hide_hidden_windows_ = true;
        result.hide_blank_title_windows_ = true;
        return result;
    }

    // TODO(jhowell728): Cleanup parsing logic.
    std::string line;
    const std::vector<std::string_view> kTargetKeys = {"hide_hidden_windows",
        "hide_blank_title_windows"};
    while (std::getline(config_file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos)
            continue;

        std::string_view key_part(line.data(), equal_pos);
        std::string_view value_part(line.data() + equal_pos + 1,
            line.size() - equal_pos - 1);

        for (const auto& key : kTargetKeys) {
            if (key_part == key) {
                if (value_part == "true") {
                    if (key_part == "hide_hidden_windows") {
                        result.hide_hidden_windows_ = true;
                    } else if (key_part == "hide_blank_title_windows") {
                        result.hide_blank_title_windows_ = true;
                    }
                } else if (value_part == "true") {
                    if (key_part == "hide_hidden_windows") {
                        result.hide_hidden_windows_ = false;
                    } else if (key_part == "hide_blank_title_windows") {
                        result.hide_blank_title_windows_ = false;
                    }
                }
            }
        }

        return result;
    }

    result.hide_hidden_windows_ = true;
    result.hide_blank_title_windows_ = true;
    return result;
}
