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

// Structures
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

struct runtime_config {
    bool use_console;
};

// Global variables
constexpr std::wstring_view FSB_REG_KEY = L"Software\\Jamie\\fsb\\Settings";
std::atomic<HINSTANCE> instance_handle;
std::atomic<HANDLE> process_handle;
std::atomic<HANDLE> process_heap_handle;
std::vector<process_window> windows;

// Function declarations
static bool create_reg();
static bool get_gui_mode();
static bool set_gui_mode(bool mode);
static void init_console();
bool EnumWindowsProc(HWND window_handle, LPARAM message_param);

int __stdcall wmain(int argc, wchar_t* argv[]) {
    // Set the process handles on application initialization
    instance_handle.store(GetModuleHandleW(nullptr));
    process_handle.store(GetCurrentProcess());
    process_heap_handle.store(GetProcessHeap());

    // RegCreateKeyExW doesn't fail if the key already exists,
    // making it safe to always run at startup
    if (!create_reg()) {
        return FSB_REGISTRY_INIT_FAILURE;
    }

    for (int i = 0; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (arg == L"--set-use-gui" || arg == L"-set-use-gui" || arg == L"/set-use-gui") {
            if (!set_gui_mode(true)) {
                return FSB_REGISTRY_SET_FAILURE;
            }

            std::wcout << L"Successfully set fsb GUI mode to ";
            return 0;
        }
    }

    if (get_gui_mode()) {
        INITCOMMONCONTROLSEX iccex;
        iccex.dwSize = sizeof(iccex);
        iccex.dwICC = ICC_WIN95_CLASSES;
        if (!InitCommonControlsEx(&iccex)) {
            return FSB_COMCTL_INIT_FAILURE;
        }
    } else {
        init_console();
        std::wcout << L"Hello, World!\r\n";
    }

    CloseHandle(process_handle.load());
    return 0;
}

static bool create_reg() {
    HKEY key_handle;
    LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, FSB_REG_KEY.data(),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
        nullptr, &key_handle, nullptr);

    if (result != ERROR_SUCCESS) {
        // Bypass the Clang-Tidy because these ancient function needs a C-Style array
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, static_cast<DWORD>(result),
            0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        const std::wstring description(buffer);
        LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to create the registry configuration for the " \
               L"application!\r\n\r\n" \
               L"Location: Line 71, fsb.exe (Main.cpp::create_reg)\r\n" \
               L"Operation: Advapi32.dll!RegCreateKeyExW\r\n" \
               L"Return value: " << result << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    DWORD value = 0;
    result = RegSetValueExW(key_handle, L"gui_mode", 0, REG_DWORD,
        reinterpret_cast<BYTE*>(&value), sizeof(DWORD));

    if (result != ERROR_SUCCESS) {
        // Bypass the Clang-Tidy because these ancient function needs a C-Style array
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, static_cast<DWORD>(result),
            0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        const std::wstring description(buffer);
        LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to create the registry configuration for the " \
               L"application!\r\n\r\n" \
               L"Location: Line 125, fsb.exe (Main.cpp::create_reg)\r\n" \
               L"Operation: Advapi32.dll!RegSetValueExW\r\n" \
               L"Return value: " << result << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    RegCloseKey(key_handle);
    return true;
}

static bool get_gui_mode() {
    DWORD value = 0, type = 0, size = sizeof(DWORD);
    LONG result = RegGetValueW(HKEY_CURRENT_USER, FSB_REG_KEY.data(), L"gui_mode",
        RRF_RT_REG_DWORD, &type, &value, &size);

    if (result != ERROR_SUCCESS || type != REG_DWORD) {
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, static_cast<DWORD>(result),
            0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        const std::wstring description(buffer);
        LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to retrieve the registry configuration for the " \
               L"application!\r\n\r\n" \
               L"Location: Line 111, fsb.exe (Main.cpp::get_gui_mode)\r\n" \
               L"Operation: Advapi32.dll!RegGetValueW\r\n" \
               L"Return value: " << result << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        exit(FSB_REGISTRY_GET_FAILURE);
    }

    return value != 0;
}

static bool set_gui_mode(bool mode) {
    HKEY key_handle;
    DWORD value = mode;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, FSB_REG_KEY.data(), 0,
        KEY_WRITE, &key_handle);

    if (result != ERROR_SUCCESS) {
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, static_cast<DWORD>(result),
            0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        const std::wstring description(buffer);
        LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to open the registry key for the " \
               L"application!\r\n\r\n" \
               L"Location: Line 145, fsb.exe (Main.cpp::set_gui_mode)\r\n" \
               L"Operation: Advapi32.dll!RegOpenKeyExW\r\n" \
               L"Return value: " << result << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    result = RegSetValueExW(key_handle, L"gui_mode", 0, REG_DWORD,
        reinterpret_cast<BYTE*>(&value), sizeof(DWORD));

    if (result != ERROR_SUCCESS) {
        wchar_t* buffer = nullptr;
        (void)FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, static_cast<DWORD>(result),
            0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        const std::wstring description(buffer);
        LocalFree(buffer);

        std::wostringstream oss;
        oss << L"An error occurred while trying to set a registry key for the " \
               L"application!\r\n\r\n" \
               L"Location: Line 173, fsb.exe (Main.cpp::set_gui_mode)\r\n" \
               L"Operation: Advapi32.dll!RegSetValueExW\r\n" \
               L"Return value: " << result << L"\r\n" \
               L"Error code: " << GetLastError() << L"\r\n" \
               L"Description: " << description.c_str();

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    RegCloseKey(key_handle);
    return true;
}

static void init_console() {
    // Check if the app was called from a console
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        // If not, we allocate a new one
        // If for some reason this function fails, it's most likely because of no resources being
        // available, which means that the detailed error message most likely won't load, so
        // show a short one and exit
        if (!AllocConsole()) {
            MessageBoxW(nullptr, L"AllocConsole error!", L"Error",
                MB_OK | MB_ICONERROR);
        }
    }

    // Open the console streams
    FILE* fp;

    if (_wfreopen_s(&fp, L"CONOUT$", L"w", stdout)) {
        std::wostringstream oss;
        oss << L"An error occurred while trying to initialize the console window.\r\n\r\n" \
               L"Location: Line 238, fsb.exe (Main.cpp::init_console)\r\n" \
               L"Operation: ucrtbase.dll!_wfreopen_s\r\n" \
               L"Error code: " << errno << L"\r\n";

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        exit(errno);
    }

    if (_wfreopen_s(&fp, L"CONOUT$", L"w", stderr)) {
        std::wostringstream oss;
        oss << L"An error occurred while trying to initialize the console window.\r\n\r\n" \
               L"Location: Line 251, fsb.exe (Main.cpp::init_console)\r\n" \
               L"Operation: ucrtbase.dll!_wfreopen_s\r\n" \
               L"Error code: " << errno << L"\r\n";

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        exit(errno);
    }

    if (_wfreopen_s(&fp, L"CONIN$", L"r", stdin)) {
        std::wostringstream oss;
        oss << L"An error occurred while trying to initialize the console window.\r\n\r\n" \
               L"Location: Line 264, fsb.exe (Main.cpp::init_console)\r\n" \
               L"Operation: ucrtbase.dll!_wfreopen_s\r\n" \
               L"Error code: " << errno << L"\r\n";

        std::wcerr << oss.str();
        MessageBoxW(nullptr, oss.str().c_str(), L"fsb - Error",
            MB_OK | MB_ICONERROR);
        exit(errno);
    }

    // Casting to void because if these fail, it won't break the application
    // Unbuffered mode
    (void)setvbuf(stdout, nullptr, _IONBF, 0);
    (void)setvbuf(stderr, nullptr, _IONBF, 0);
    (void)setvbuf(stdin, nullptr, _IONBF, 0);

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

    // Since if the console doesn't allocate, the app exits, this should never fail, So cast to void
    (void)SetConsoleTitleW(L"Full Screen Borderless");
}

bool EnumWindowsProc(HWND window_handle, LPARAM message_param) {
    UNREFERENCED_PARAMETER(message_param);

    DWORD process_id;
    GetWindowThreadProcessId(window_handle, &process_id);

    wchar_t buffer[256];
    GetWindowTextW(window_handle, buffer, std::size(buffer));

    std::wstring title(buffer);

    if (IsWindowVisible(window_handle) && !title.empty()) {
        windows.push_back({process_id, window_handle, title});
    }

    return true;
}
