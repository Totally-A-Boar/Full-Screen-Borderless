#include <Windows.h>
#include <iostream>
#include <vector>
#include <conio.h>

// Structure to store process information
struct ProcessWindow {
    DWORD processId;
    HWND hwnd;
    std::string title;
};

// Global list of windows
std::vector<ProcessWindow> windows;

// Forward declaration of functions
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam); // Callback function for EnumWindows
void FullScreenWindow(HWND hwnd);
void ShowMenu();

int main() {
    EnumWindows(EnumWindowsProc, 0);

    if (windows.empty()) {
        std::cout << "No foreground processes found!" << std::endl;
        return 1;
    }

    ShowMenu();

    return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));

    if (IsWindowVisible(hwnd) && strlen(title) > 0) {
        windows.push_back({ pid, hwnd, title });
    }

    return TRUE;
}

void FullScreenWindow(HWND hwnd) {
    SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
}

void ShowMenu() {
    int selected = 0;
    char key;

    while (true) {
        std::cout << "\033c"; // Clear the screen using the ANSI escape code
        std::cout << "Select a process to make fullscreen:" << std::endl << std::endl;

        for (size_t i = 0; i < windows.size(); i++) {
            if (i == selected) {
                std::cout << " > ";
            } else {
                std::cout << "  ";
            }

            std::cout << windows[i].title << "(PID: " << windows[i].processId << ")" << std::endl;
        }

        key = _getch();
        if (key == 72 && selected > 0) selected--;  // Up arrow
        if (key == 80 && selected < windows.size() - 1) selected++;  // Down arrow
        if (key == 27) exit(0);
        if (key == '\r') {  // Enter key
			FullScreenWindow(windows[selected].hwnd);
			break;
		}
    }
}