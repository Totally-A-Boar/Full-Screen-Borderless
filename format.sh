#!/bin/bash

# Ensure clang-format is available
if ! command -v clang-format &> /dev/null
then
  echo "clang-format could not be found! Please install it or add it to your path."
  echo "" # Line break
  echo "On Windows, this can most likely be done via your IDE. If you are using an editor, such as" \
       "Visual Studio Code, or Vim, than you must install a plugin. Otherwise, use a Linux terminal for Windows," \
       "Such as MSYS2 or, WSL (Windows Subsystem for Linux)."
  echo ""
  echo "On MSYS2, this be done by typing: 'pacman -S mingw-w64-x86_64-clang-tools-extra'"
  echo ""
  echo "On Cygwin, you have to use the Cygwin installer, as there is no CLI package manager."
  echo ""
  echo "If you are on Linux, keep in mind, fsb is for Windows, and will not cross-compile to Linux systems, however," \
       "The command for installing clang-format on Ubuntu is: 'sudo apt install clang-format'"
  exit 1
fi

# Format the files
clang-format -i src/*.cc
clang-format -i src/*.hh

echo "Formatting has completed."