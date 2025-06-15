#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <atomic>
#include <string>
#include <string_view>

// Structures
//! Information used by the program belonging to a process's window.
struct ProcessWindow
{
    //! Process ID belonging to the target. Mainly used for display purposes
    DWORD m_processId;

    //! Window handle belonging to the target. This is the main field that matters, because,
    //! This is how we maximize the window
    HWND m_hwnd;

    //! Title of the target window. Mainly used for display purposes
    std::wstring m_title;
};

struct RuntimeConfig
{
    bool m_useConsole;
};

// Global variables
constexpr const std::wstring g_regKey = L"Software\\Jamie\\fsb\\Settings";
std::atomic<HINSTANCE> g_hInstance;
std::atomic<HANDLE> g_hProcess;
std::atomic<HANDLE> g_hProcessHeap;

// Function declarations
void create_reg();
template <typename T>
bool set_reg_value(std::wstring_view valueName, T value);
template <typename T>
T get_reg_value(std::wstring_view valueName);

int __stdcall wmain(int argc, wchar_t* argv[])
{
    g_hInstance.store(GetModuleHandleW(nullptr));
    g_hProcess.store(GetCurrentProcess());
    g_hProcessHeap.store(GetProcessHeap());

    for (int i = 0; i < argc; ++i)
    {
        const std::wstring arg = argv[i];
        if (arg == L"--set-use-gui")
        {

        }
    }

    return 0;
}

void create_reg()
{
    HKEY hKey;
    LSTATUS lStatus;

    lStatus = RegCreateKeyExW(HKEY_CURRENT_USER, g_regKey.c_str(), 0,
        nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
        nullptr, &hKey, nullptr);

    if (lStatus != ERROR_SUCCESS)
    {

    }
}