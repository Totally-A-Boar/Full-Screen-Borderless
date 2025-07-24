#!/bin/bash

if ! command -v clang-format &> /dev/null; then
  echo "clang-format could not be found! Please install it or add it to your path."
  echo
  echo "On Windows, this can most likely be done via your IDE."
  echo "If you are using an editor such as Visual Studio Code or Vim, then you must install a plugin."
  echo "Otherwise, use a Linux terminal for Windows, such as MSYS2 or WSL (Windows Subsystem for Linux)."
  echo
  echo "On MSYS2, this can be done by typing: 'pacman -S mingw-w64-x86_64-clang-tools-extra'"
  echo
  echo "On Cygwin, you have to use the Cygwin installer, as there is no CLI package manager."
  echo
  echo "If you are on Linux, fsb is for Windows and will not cross-compile to Linux systems."
  echo "The command for installing clang-format on Ubuntu is: 'sudo apt install clang-format'"
  exit 1
fi

if compgen -G "src/*.cc" > /dev/null || compgen -G "src/*.h" > /dev/null; then
  clang-format -i src/*.cc 2> /dev/null
  clang-format -i src/*.h  2> /dev/null
  echo "Formatting has completed."
else
  echo "No .cc or .h files found in src/ directory. Make sure to run git pull again!"
fi