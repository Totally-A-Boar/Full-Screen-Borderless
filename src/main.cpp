#include <Windows.h>
#include <iostream>

// Structures
//! Information used by the program belonging to a process's window.
struct ProcessWindow
{
    //! Process ID belonging to the target. Mainly used for dislay purposes
    DWORD m_processId;

    //! Window handle belonging to the target. This is the main field that matters, because,
    //! This is how we maximize the window
    HWND m_hwnd;

    //! Title of the target window. Mainly used for display purposes
    std::wstring m_title;
};

// Global variables

// Function declarations


int __stdcall wmain(int argc, wchar_t* argv[])
{
    return 0;
}