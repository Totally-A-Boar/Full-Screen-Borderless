cmake_minimum_required(VERSION 3.31)
project(
        fsb
        LANGUAGES CXX
        VERSION 2.0.0
        DESCRIPTION "Window management application for Windows."
)

set(CMAKE_CXX_STANDARD 17)

add_executable(fsb src/main.cpp)

if (!WIN32)
    message(FATAL_ERROR "fsb is not supported on non-Windows platforms!")
endif ()

set(CMAKE_GENERATOR_PLATFORM Win32)

# Compiler switch
if (MSVC)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/asm")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/obj")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(fsb PRIVATE
                _DEBUG
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                /JMC
                /permissive-
                /Zc:wchar_t
                /GS
                /W4
                /ZI
                /Od
                /sdl
                /Zc:inline
                /fp:precise
                /errorReport:prompt
                /WX
                /Zc:forScope
                /RTC1
                /Gd
                /MDd
                /FC
                /EHsc
                /nologo
                /diagnostics:column
                /Fa${CMAKE_BINARY_DIR}/asm/
                /Fo${CMAKE_BINARY_DIR}/obj/
                /Fp${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pch
        )

        target_link_options(fsb PRIVATE
                /DEBUG
                /INCREMENTAL
                /MANIFEST
                /NXCOMPAT
                /DYNAMICBASE
                /SUBSYSTEM:WINDOWS
                /ENTRY:wmainCRTStartup
                /PDB:${CMAKE_BINARY_DIR}/fsb.pdb
                /ILK:${CMAKE_BINARY_DIR}/fsb.ilk
                /NOLOGO
                /ManifestFile:resources/fsb.manifest
                /MACHINE:X64
                /ERRORREPORT:PROMPT
                /TLBID:1
                kernel32.lib user32.lib advapi32.lib comctl32.lib
        )

    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_definitions(fsb PRIVATE
                NDEBUG
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                /W4
                /O2
                /GL
                /Gy
                /Zc:wchar_t
                /GS
                /Gw
                /Zc:inline
                /fp:precise
                /permissive-
                /WX-
                /Zc:forScope
                /Gd
                /MD
                /FC
                /EHsc
                /nologo
                /diagnostics:column
                /Fa${CMAKE_BINARY_DIR}/asm
                /Fo${CMAKE_BINARY_DIR}/obj
                /Fp${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pch
        )

        target_link_options(fsb PRIVATE
                /LTCG
                /MANIFEST
                /NXCOMPAT
                /DYNAMICBASE
                /SUBSYSTEM:WINDOWS
                /ENTRY:wmainCRTStartup
                /OPT:REF
                /OPT:ICF
                /PDB:${CMAKE_BINARY_DIR}/fsb.pdb
                /PGD:${CMAKE_BINARY_DIR}/fsb.pgd
                /NOLOGO
                /ManifestFile:resources/fsb.manifest
                /MACHINE:X64
                /ERRORREPORT:PROMPT
                /TLBID:1
                kernel32.lib user32.lib advapi32.lib comctl32.lib
        )
    endif ()

elseif (MINGW)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")

        target_compile_definitions(fsb PRIVATE
                _DEBUG
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                -Wall               # Enable most warnings
                -Wextra             # More warnings
                -Wpedantic          # Pedantic compliance
                -g                  # Generate debug info
                -O0                 # No optimizations
                -std=c++17          # Use C++17 (or as needed)
                -fno-omit-frame-pointer  # Required for stack traces
                -fdiagnostics-color=always  # Color diagnostics
                -fexceptions        # Enable exception handling (default)
                -finput-charset=UTF-8
                -fexec-charset=UTF-16LE
                -D_GLIBCXX_USE_CXX11_ABI=1  # Optional: Use C++11 ABI
        )

        target_link_options(fsb PRIVATE
                -Wl,--enable-auto-import
                -Wl,--nxcompat       # DEP
                -Wl,--dynamicbase    # ASLR
                -static-libgcc       # Optional: avoid DLL dependency
                -static-libstdc++    # Optional: avoid DLL dependency
                -mwindows            # Use Windows subsystem
                -m32                 # 32-bit build; omit for 64-bit
                -lkernel32 -luser32 -ladvapi32 -lcomctl32
                -g                   # Include symbols in output
        )

    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")

        target_compile_definitions(fsb PRIVATE
                NDEBUG
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                -O2                 # Optimize for speed
                -march=native      # Use native CPU features (optional)
                -Wall
                -Wextra
                -Wpedantic
                -DNDEBUG           # Disable assertions
                -std=c++17         # C++ standard (adjust as needed)
                -fno-exceptions    # Optional: strip exception support
                -fno-rtti          # Optional: disable RTTI
                -fmerge-all-constants
                -fomit-frame-pointer
                -fno-ident         # Don't include version string in output
                -finput-charset=UTF-8
                -fexec-charset=UTF-16LE
        )

        target_link_options(fsb PRIVATE
                -Wl,--strip-all            # Strip all symbols
                -Wl,--nxcompat             # DEP
                -Wl,--dynamicbase          # ASLR
                -static-libgcc             # Avoid DLL dependencies
                -static-libstdc++          # Avoid DLL dependencies
                -s                         # Strip symbol table and relocation info
                -mwindows                  # Use Windows GUI subsystem
                -m32                       # Build 32-bit binary (omit if 64-bit)
                -lkernel32 -luser32 -ladvapi32 -lcomctl32
        )

    endif ()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Debug")

    target_compile_definitions(fsb PRIVATE
            _DEBUG
            _WINDOWS
            _UNICODE
            UNICODE
            WIN32_LEAN_AND_MEAN
    )

    target_compile_options(fsb PRIVATE
            -fms-extensions               # Enable MS-specific extensions
            -fms-compatibility            # MSVC compatibility mode
            -Wno-invalid-token-paste      # Silence MSVC macro paste warnings
            -Wall                         # All warnings
            -Wextra                       # Extra warnings
            -Wpedantic                    # Pedantic warnings
            -WX                           # Warnings as errors
            -g                            # Debug info
            -O0                           # No optimization
            -fstack-protector-strong      # Like /GS
            -fsized-deallocation          # Modern C++
            -fno-rtti                     # Match `/GR-` if desired
            -fexceptions                  # Like /EHsc
            -Xclang -fcxx-exceptions      # Explicit exception model
            -std=c++17                    # Adjust as needed
            -D_CRT_SECURE_NO_WARNINGS     # Suppress CRT warnings if needed
            -fcolor-diagnostics           # Colored output
            -fansi-escape-codes
            -m32                          # Target X64
            -MD                           # Generate dependency file (optional)
    )

    target_link_options(fsb PRIVATE
            -Wl,/DEBUG
            -Wl,/INCREMENTAL
            -Wl,/MANIFEST
            -Wl,/NXCOMPAT
            -Wl,/DYNAMICBASE
            -Wl,/SUBSYSTEM:WINDOWS
            -Wl,/ENTRY:wmainCRTStartup
            -Wl,/PDB:"${CMAKE_BINARY_DIR}/fsb.pdb"
            -Wl,/PGD:"${CMAKE_BINARY_DIR}/fsb.pgd"
            -Wl,/ILK:"${CMAKE_BINARY_DIR}/fsb.ilk"
            -Wl,/NOLOGO
            -Wl,/MANIFESTFILE:"resources/fsb.manifest"
            -Wl,/MACHINE:X64
            -Wl,/ERRORREPORT:PROMPT
            -Wl,/TLBID:1
            kernel32.lib user32.lib advapi32.lib comctl32.lib
    )

else ()

    message(FATAL_ERROR "Error: The compiler you are using is not supported by fsb.")

endif ()