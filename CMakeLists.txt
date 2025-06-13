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

# Construct the output path
get_target_property(OUTPUT_NAME fsb OUTPUT_NAME)
if (NOT OUTPUT_NAME)
    endif()

# Compiler switch
if (MSVC)
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
                /Wall
                /ZI
                /Od
                /sdl
                /Zc:inline
                /fp:precise
                /errorReport:prompt
                /WX-
                /Zc:forScope
                /RTC1
                /Gd
                /MDd
                /FC
                /EHsc
                /nologo
                /diagnostics:column
                /Fa"${CMAKE_BINARY_DIR}\\asm"
                /Fo"${CMAKE_BINARY_DIR}\\obj"
                /Fp"${CMAKE_BINARY_DIR}\\${CMAKE_PROJECT_NAME}.pch"
        )

        target_link_options(fsb PRIVATE

        )
    endif ()
endif ()