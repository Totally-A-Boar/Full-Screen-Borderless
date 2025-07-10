cmake_minimum_required(VERSION 3.31)
project(
        fsb
        LANGUAGES CXX
        VERSION 2.0.0
        DESCRIPTION "Window management application for Windows."
)

set(CMAKE_CXX_STANDARD 17)

add_executable(fsb
        src/main.cc
        src/console.cc
)

if (!WIN32)
    message(FATAL_ERROR "fsb is not supported on non-Windows platforms!")
endif ()

set(CMAKE_GENERATOR_PLATFORM Win32)

target_include_directories(fsb PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# Compiler switch
if (MSVC)
    # Always create this directory as the compiler will fail if it doesn't exist.
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/asm")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/obj")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Debug section of MSVC compiler args
        target_compile_definitions(fsb PRIVATE
                _DEBUG              # Include debug specific functions and conditions
                _WINDOWS            # Built for Windows
                _UNICODE            # Use Wide strings and functions
                UNICODE             # Ditto
                WIN32_LEAN_AND_MEAN # Exclude rarely-used APIs from Windows.h
        )

        target_compile_options(fsb PRIVATE
                /JMC                # Just My Code debugging
                /permissive-        # C++ STL conforming code
                /Zc:wchar_t         # Treat wchar_t as a native type
                /utf-8              # Fix UTF-8 output error; char type strings interpreted as Windows-1252
                /GS                 # Buffer security checks
                /W4                 # Warning level 4
                /ZI                 # Debug information format
                /Od                 # Disable optimizations
                /sdl                # Security development lifecycle
                /Zc:inline          # Remove unreferenced inline functions from compiled binary
                /fp:precise         # Precise float-point semantics
                /errorReport:prompt # Report build errors immediately
                /WX                 # Treat warnings as errors
                /Zc:forScope        # for loop variables are only for the scope
                /RTC1               # Enable runtime error checks
                /Gd                 # __cdecl calling conventions
                /MDd                # Multi-threaded debug DLL CRT (ucrtbased.dll, vcruntime140d.dll, etc)
                /FC                 # Display full path in diagnostics
                /EHsc               # C++ exceptions with stack unwinding & extern "C" functions never throw exceptions
                /nologo             # Suppress Microsoft compiler logo
                /diagnostics:column # Enable column number diagnostics
                /Fa${CMAKE_BINARY_DIR}/asm/ # Assembly listing output directory
                /Fo${CMAKE_BINARY_DIR}/obj/ # Object file output directory
                /Fp${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pch # Precompiled header
        )

        target_link_options(fsb PRIVATE
                /DEBUG              # Generate debug symbols
                /INCREMENTAL        # Incrementally link all dependencies
                /MANIFEST           # Embed a manifest
                /NXCOMPAT           # Data execution prevention
                /DYNAMICBASE        # Randomizes the base of the executable image at load time
                /SUBSYSTEM:CONSOLE  # Console subsystem
                /PDB:${CMAKE_BINARY_DIR}/fsb.pdb # PDB file output
                /ILK:${CMAKE_BINARY_DIR}/fsb.ilk # Incremental linking file output
                /NOLOGO             # Suppress Microsoft linker logo
                /ManifestFile:resources/fsb.manifest # Path to the manifest to embed
                /MACHINE:X64        # Compile for 64-bit (x86_64)
                /ERRORREPORT:PROMPT # Is deprecated, but I still put it here, it is safe to ignore this
                /TLBID:1            # Resource ID for TypeLib
                kernel32.lib user32.lib advapi32.lib comctl32.lib # Static libraries of the dependencies used
        )

    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        # Mostly same as above, but enables optimizations and disables debug macros
        target_compile_definitions(fsb PRIVATE
                NDEBUG              # Disable debug macros and functions
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                /W4
                /O2                 # Optimize the program for speed
                /GL                 # Whole program optimization
                /Gy                 # Function level linking
                /Zc:wchar_t
                /utf-8
                /GS
                /Gw                 # Optimize global variable access
                /Zc:inline
                /fp:precise
                /permissive-
                /WX-                # Do not treat warnings as errors
                /Zc:forScope
                /Gd
                /MD                 # Multi-threaded DLL CRT (ucrtbase.dll, vcruntime140.dll, etc)
                /FC
                /EHsc
                /nologo
                /diagnostics:column
                /Fa${CMAKE_BINARY_DIR}/asm
                /Fo${CMAKE_BINARY_DIR}/obj
                /Fp${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pch
        )

        target_link_options(fsb PRIVATE
                /LTCG               # Link-time code generation
                /MANIFEST
                /NXCOMPAT
                /DYNAMICBASE
                /SUBSYSTEM:CONSOLE
                /OPT:REF            # Eliminate unused functions & data
                /OPT:ICF            # Remove duplicate COMDATs
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
        # Exact same as MSVC, just with GCC syntax
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
        )

        target_link_options(fsb PRIVATE
                -Wl,--enable-auto-import
                -Wl,--nxcompat       # DEP
                -Wl,--dynamicbase    # ASLR
                -mconsole            # Use console subsystem
                -m32                 # 32-bit build; omit for 64-bit
                -lkernel32 -luser32 -ladvapi32 -lcomctl32
                -g                   # Include symbols in output
        )

    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        # Exact same as before
        target_compile_definitions(fsb PRIVATE
                NDEBUG
                _WINDOWS
                _UNICODE
                UNICODE
                WIN32_LEAN_AND_MEAN
        )

        target_compile_options(fsb PRIVATE
                -O2                 # Optimize for speed
                -Wall
                -Wextra
                -Wpedantic
                -DNDEBUG           # Disable assertions
                -std=c++17         # C++ standard
                -fmerge-all-constants
                -fomit-frame-pointer
                -finput-charset=UTF-8
                -fexec-charset=UTF-16LE
        )

        target_link_options(fsb PRIVATE
                -Wl,--strip-all
                -Wl,--nxcompat
                -Wl,--dynamicbase
                -s                         # Strip symbol table and relocation info
                -mconsole
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
            -m64                          # Target X64
            -MD
    )

    # Almost the exact same as the MSVC version
    target_link_options(fsb PRIVATE
            -Wl,/DEBUG
            -Wl,/INCREMENTAL
            -Wl,/MANIFEST
            -Wl,/NXCOMPAT
            -Wl,/DYNAMICBASE
            -Wl,/SUBSYSTEM:CONSOLE
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

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Release")

    target_compile_definitions(fsb PRIVATE
            NDEBUG
            _WINDOWS
            _UNICODE
            UNICODE
            WIN32_LEAN_AND_MEAN
    )

    target_compile_options(fsb PRIVATE
            -fms-extensions
            -fms-compatibility
            -Wno-invalid-token-paste
            -Wall
            -Wextra
            -Wpedantic
            -WX
            -O2                           # Optimization level for release
            -fstack-protector-strong
            -fsized-deallocation
            -fno-rtti
            -fexceptions
            -Xclang -fcxx-exceptions
            -std=c++17
            -D_CRT_SECURE_NO_WARNINGS
            -fcolor-diagnostics
            -fansi-escape-codes
            -m64
            -MD
    )

    target_link_options(fsb PRIVATE
            -Wl,/INCREMENTAL:NO          # Disable incremental linking for release
            -Wl,/MANIFEST
            -Wl,/NXCOMPAT
            -Wl,/DYNAMICBASE
            -Wl,/SUBSYSTEM:CONSOLE
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