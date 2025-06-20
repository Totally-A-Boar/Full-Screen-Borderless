#include <Windows.h>
#include <CommCtrl.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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
//! Information used by the program belonging to a process's window.
struct process_window {
    //! Process ID belonging to the target. Mainly used for display purposes
    uint32_t process_id;

    //! Window handle belonging to the target. This is the main field that matters, because,
    //! This is how we maximize the window
    HWND window_handle;

    //! Title of the target window. Mainly used for display purposes
    std::wstring title;
};

// Global variables
constexpr std::wstring_view FSB_REG_KEY = L"Software\\Jamie\\fsb\\Settings";
std::atomic<HINSTANCE> instance_handle;
std::atomic<HANDLE> process_handle;
std::atomic<HANDLE> process_heap_handle;
std::vector<process_window> windows;

// Functions
void init_console() {
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

    // Since if the console doesn't allocate, the app exits, this should never fail, So cast to
    // void
    (void)SetConsoleTitleW(L"Full Screen Borderless");
}

int EnumWindowsProc(HWND window_handle, LPARAM message_param) {
    UNREFERENCED_PARAMETER(message_param);

    DWORD process_id;
    GetWindowThreadProcessId(window_handle, &process_id);

    wchar_t buffer[256];
    GetWindowTextW(window_handle, buffer, std::size(buffer));

    std::wstring title(buffer);

    if (IsWindowVisible(window_handle) && !title.empty()) {
        windows.push_back({process_id, window_handle, title});
    }

    return 1; // returning true
}

void fullscreen_window(HWND window_handle) {
    SetWindowLongW(window_handle, GWL_STYLE, WS_VISIBLE);
    SetWindowPos(window_handle, HWND_TOP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED);
}

void clear_console() {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;

    if (!GetConsoleScreenBufferInfo(console_handle, &csbi)) return;

    DWORD cell_count = csbi.dwSize.X * csbi.dwSize.Y;

    COORD home_coords = {0, 0};

    FillConsoleOutputCharacterW(console_handle, L' ', cell_count, home_coords, &count);
    FillConsoleOutputAttribute(console_handle, csbi.wAttributes, cell_count, home_coords, &count);
    SetConsoleCursorPosition(console_handle, home_coords);
}

void show_console_menu() {
    int selected = 0;

    while (true) {
        clear_console(); // Clear the screen
        std::wcout << L"Select a process to fullscreen:\r\n\r\n";

        for (size_t i = 0; i < windows.size(); ++i) {
            if (i == selected) {
                std::wcout << L" * ";
            } else {
                std::wcout << L"   ";
            }

            std::wostringstream oss;
            oss << windows[i].title << L" (Process Id: " << windows[i].process_id << ")\r\n";
            std::wcout << oss.str();
        }

        std::wcout << L"\r\nPress 'r' to refresh\r\n";
        std::wcout << L"Press 'Q' or escape to quit.";

        wchar_t key = _getwch();
        if (key == 0x1B || key == L'Q' || key == L'q') {
            exit(0);
        }
        // Enter key
        if (key == 0x0D) {
            fullscreen_window(windows[selected].window_handle);
            exit(0);
        }
        // R key
        if (key == L'R' || key == L'r') {
            windows.clear();
            EnumWindows(EnumWindowsProc, 0);
        }
        // Up arrow
        if (key == 72) {
            if (selected > 0) selected--;
            else selected = static_cast<int>(windows.size()) - 1;
        }
        // Down arrow
        if (key == 80) {
            if (selected < static_cast<int>(windows.size()) - 1) selected++;
            else selected = 0;
        }
    }
}

} // namespace fsb

int __stdcall wmain() {
    // Set the process handles on application initialization
    fsb::instance_handle.store(GetModuleHandleW(nullptr));
    fsb::process_handle.store(GetCurrentProcess());
    fsb::process_heap_handle.store(GetProcessHeap());

    EnumWindows(fsb::EnumWindowsProc, 0);
    if (fsb::windows.empty()) {
        std::wostringstream oss;
        oss << L"You have launched fsb with no foreground windows! " \
               L"Please relaunch with at least one foreground window.";

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - No foreground windows!",
            MB_OK | MB_ICONWARNING);
        return FSB_NO_FOREGROUND_WINDOWS;
    }

    fsb::init_console();

    CloseHandle(fsb::process_handle.load());
    return 0;
}

