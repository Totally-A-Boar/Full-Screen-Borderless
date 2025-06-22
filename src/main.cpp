#include <Windows.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "../third_party/colors/include/colors/colors.hpp"

// Custom error codes for specific cases
#define FSB_NO_ERROR              0x00000000 // No error
#define FSB_GENERIC_FAILURE       0xFB000001 // Generic failure fallback code
#define FSB_REGISTRY_INIT_FAILURE 0xFB000002 // Initializing the registry key failed
#define FSB_REGISTRY_SET_FAILURE  0xFB000003 // Setting a registry value failed
#define FSB_REGISTRY_GET_FAILURE  0xFB000004 // Getting a registry value failed
#define FSB_COMCTL_INIT_FAILURE   0xFB000005 // Initializing common controls failed
#define FSB_CONSOLE_INIT_FAILURE  0xFB000006 // Error initializing the console
#define FSB_NO_FOREGROUND_WINDOWS 0xFB000007 // FSB was launched with no foreground windows

namespace fsb {
//! @brief Holds information about a process and its associated window.
//!
//! This structure is the main structure used within the application to manage the window's full
//! screen properties, and the window's title and process ID for display to the user
struct process_window {
    //! The process ID associated with the window/application
    uint32_t process_id;

    //! Handle to the window. This is the actual property used by the application to apply the
    //! full-screen property
    HWND window_handle;

    //! Title of the window to display in the interface
    std::wstring title;
};

// Global variable(s)
//! Vector of all foreground windows. Contains display data and window handle to full screen it.
std::vector<process_window> windows;

// Functions
//! @brief Initializes the console for proper UTF-8 I/O.
//!
//! This function sets up the standard console streams for UTF-8 output and sets the console in
//! unbuffered mode to ensure proper handling of Unicode characters from wcout (UTF-16) in the
//! console output streams.
//!
//! It also sets the console to the proper code page and sets the title for cleanliness and to look
//! as proper as possible.
//!
//! @note this function casts the calls within it because it is non-critical and shouldn't fail
//! unless there are larger problems than just the one call failing
//! (e.g. the system is out of memory).
void init_console() {
    // Hide the cusor
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor_info;

    GetConsoleCursorInfo(console_handle, &cursor_info);
    cursor_info.dwSize = sizeof(cursor_info);
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(console_handle, &cursor_info);

    // Casting to void because if these fail, it won't break the application
    // Unbuffered mode
    (void)setvbuf(stdout, nullptr, _IONBF, 0);
    (void)setvbuf(stderr, nullptr, _IONBF, 0);
    (void)setvbuf(stdin, nullptr, _IONBF, 0);

    // Active code page needs to be set to UTF-8 for proper Unicode output
    (void)SetConsoleOutputCP(CP_UTF8);

    // UTF-8 displays without error on all consoles and has more characters than basic ASCII,
    // however, Unicode characters will not display with this.
    // UTF-8 on Windows is really hard because of Microsoft's obsession with UTF-16 when the
    // Windows API was created. If I did try to use UTF-8 as the actual encoding in everything
    // It would be wildly inconsistent and have a conversion every other line.
    // We don't set it to UTF-16 because i t  w o u l d  o u t p u t  l i k e  t h i s on every
    // console that doesn't support UTF-16 output.
    (void)_setmode(_fileno(stdout), _O_U8TEXT);
    (void)_setmode(_fileno(stderr), _O_U8TEXT);
    (void)_setmode(_fileno(stdin), _O_U8TEXT);

    // Set the title and cast to void because it shouldn't fail unless there are memory issues
    // which would probably crash the app or the OS entirely.
    (void)SetConsoleTitleW(L"Full Screen Borderless");
}

//! @brief Callback function used with EnumWindows to collect all windows currently running on the
//! system
//!
//! This function is called by the Windows API for and contains custom logic to loop through all
//! windows and add it to the vector for the process_window struct if it is visible.
//!
//! @param window_handle Handle to the current window that is being looped through. Is used to
//! obtain the title, process ID, and saves the handle itself
//! @param message_param Unused parameter; required by the API.
int EnumWindowsProc(HWND window_handle, LPARAM message_param) {
    // Function signature has to match, but I don't use the LPARAM, so use UNREFERENCED_PARAMETER
    // to avoid the compiler complaining it is unreferenced
    UNREFERENCED_PARAMETER(message_param);

    if (window_handle == nullptr || !IsWindow(window_handle)) {
        return 1; // Continue enumeration, but skip this iteration
    }

    // Saved for display information
    DWORD process_id = 0;
    if (GetWindowThreadProcessId(window_handle, &process_id) == 0) {
        wchar_t* buffer = nullptr; // It allocates the buffer so make it a pointer
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, GetLastError(),
            0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

        std::wstring description(buffer ? buffer : L"Unknown error");
        if (buffer) LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to get the process ID for a window.\r\n\r\n" \
               L"Location: Line 103, fsb.exe (Main.cpp::EnumWindowsProc)\r\n" \
               L"Operation: User32.dll!GetWindowThreadProcessId\r\n" \
               L"Return value: 0x0" << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        return 1;
    }

    // Ditto
    wchar_t buffer[256];
    if (GetWindowTextW(window_handle, buffer, std::size(buffer)) == 0) {
        if (GetLastError() != 0) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::wstring description(err_buffer ? err_buffer : L"Unknown error");
            if (err_buffer) LocalFree(err_buffer);

            std::wostringstream oss;
            oss << L"An error occurred while trying to get the title for a window.\r\n\r\n" \
                   L"Location: Line 128, fsb.exe (Main.cpp::EnumWindowsProc)\r\n" \
                   L"Operation: User32.dll!GetWindowTextW\r\n" \
                   L"Return value: 0x0" << L"\r\n" \
                   L"Error code: 0x" << std::hex << GetLastError() << L"\r\n" \
                   L"Description: " << description.c_str();

            std::wcerr << oss.str();
            return 1;
        }
    }

    // Put the buffer into a proper string and check if the window is a foreground window
    if (std::wstring title(buffer); IsWindowVisible(window_handle) && !title.empty()) {
        // If so, add it to the vector
        windows.push_back({process_id, window_handle, title});
    }

    return 1;  // returning true
}

//! @brief Sets a window's properties to be in full-screen mode.
//!
//! This function will modify a given window to remove borders and resize it to cover the bounds of
//! the screen.
//!
//! @param window_handle Handle to the window that is to be modified.
void fullscreen_window(HWND window_handle) {
    // Make sure the window is visible
    SetWindowLongW(window_handle, GWL_STYLE, WS_VISIBLE);
    // Now set the window's properties to cover the full screen
    SetWindowPos(window_handle, HWND_TOP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED);
}

//! @brief Clears the entire console screen and resets the cursor
//!
//! This function retrieves the handle to the stdout stream and fills the entire consoles with
//! spaces to simulate a clear console. The function then sets the cursor position to (0,0)
void clear_console() {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;

    if (!GetConsoleScreenBufferInfo(console_handle, &csbi)) return;

    DWORD cell_count = csbi.dwSize.X * csbi.dwSize.Y;

    COORD home_coords = {0, 0};

    (void)FillConsoleOutputCharacterW(console_handle, L' ', cell_count, home_coords,
        &count);
    (void)FillConsoleOutputAttribute(console_handle, csbi.wAttributes, cell_count, home_coords,
        &count);
    (void)SetConsoleCursorPosition(console_handle, home_coords);
}

void show_console_menu() {
    int selected_x = 0;

    while (true) {
        clear_console(); // Clear the screen
        std::wcout << L"Select a process to fullscreen:\r\n\r\n";

        for (size_t i = 0; i < windows.size(); ++i) {
            if (i == selected_x) {
                // For some reason colors.hpp doesn't work on Wide chars
                std::wcout << L"   ";
                std::cout << colors::grey << colors::on_white;
            } else {
                std::cout << colors::reset;
                std::wcout << L"   ";
            }

            std::wostringstream oss;
            oss << windows[i].title << L" (Process Id: " << windows[i].process_id << ")\r\n";
            std::wcout << oss.str();
            std::cout << colors::reset;
        }

        // Check how many = signs to fill a row on the screen
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console_handle == INVALID_HANDLE_VALUE) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::wstring description(err_buffer ? err_buffer : L"Unknown error");
            if (err_buffer) LocalFree(err_buffer);

            std::wostringstream oss;
            oss << L"An error occurred while trying to get the handle to the console.\r\n\r\n" \
                   L"Location: Line 228, fsb.exe (Main.cpp::show_console_menu)\r\n" \
                   L"Operation: Kernel32.dll!GetStdHandle\r\n" \
                   L"Error code: 0x" << std::hex << GetLastError() << L"\r\n" \
                   L"Description: " << description.c_str();

            std::wcerr << oss.str();
            exit(FSB_GENERIC_FAILURE);
        }

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(console_handle, &csbi)) {
            wchar_t* err_buffer = nullptr; // It allocates the buffer so make it a pointer
            (void)FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(),
                0, reinterpret_cast<wchar_t*>(&err_buffer), 0, nullptr);

            std::wstring description(err_buffer ? err_buffer : L"Unknown error");
            if (err_buffer) LocalFree(err_buffer);

            std::wostringstream oss;
            oss << L"An error occurred while trying to get the console buffer info.\r\n\r\n" \
                   L"Location: Line 252, fsb.exe (Main.cpp::show_console_menu)\r\n" \
                   L"Operation: Kernel32.dll!GetConsoleScreenBufferInfo\r\n" \
                   L"Return value: false" << L"\r\n" \
                   L"Error code: 0x" << std::hex << GetLastError() << L"\r\n" \
                   L"Description: " << description.c_str();

            std::wcerr << oss.str();
        }

        int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        int cursor_y = csbi.dwCursorPosition.Y + csbi.srWindow.Top;

        int lines_to_move = (height - 2) - cursor_y;

        for (int i = 0; i < lines_to_move; ++i) {
            std::wcout << L"\r\n";
        }

        std::wstring row(width, L'=');

        std::wcout << row << L"\r\n";
        std::wcout << L"[ Q ]uit [ R ]efresh [ Enter ] Apply";

        wchar_t key = _getwch();
        if (key == 0x1B || key == L'Q' || key == L'q') {
            exit(0);
        }
        // Enter key
        if (key == L'\r') {
            fullscreen_window(windows[selected_x].window_handle);
            exit(0);
        }
        // R key
        if (key == L'R' || key == L'r') {
            windows.clear();
            EnumWindows(EnumWindowsProc, 0);
        }
        // Up arrow
        if (key == 72) {
            if (selected_x > 0) selected_x--;
            else selected_x = static_cast<int>(windows.size()) - 1;
        }
        // Down arrow
        if (key == 80) {
            if (selected_x < static_cast<int>(windows.size()) - 1) selected_x++;
            else selected_x = 0;
        }
    }
}

} // namespace fsb

int __stdcall wmain() {
    // Set up the console for proper I/O
    fsb::init_console();

    EnumWindows(fsb::EnumWindowsProc, 0);
    if (fsb::windows.empty()) {
        std::wostringstream oss;
        oss << L"You have launched fsb with no foreground windows! " \
               L"Please relaunch with at least one foreground window.";
        std::wcerr << oss.str();
        return FSB_NO_FOREGROUND_WINDOWS;
    }

    fsb::show_console_menu();

    return 0;
}

