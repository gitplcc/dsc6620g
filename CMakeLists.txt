cmake_minimum_required(VERSION 4.3.0...4.4.4)

project(dsc6620g LANGUAGES CXX)

# --- 1. CPM automatic downloading & including ---
set(CPM_DOWNLOAD_VERSION 0.43.2)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CMAKE_BINARY_DIR}/cmake/CPM.cmake"
    )
endif()
include("${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

# --- 2. Centralized binary management (Solution to the error "file not found xxx.dll") ---
if(WIN32)
# Redirects executables (*.exe) and dynamic libraries (*.dll) to the same folder
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# --- 3. Dependency management with CPM ---

# A. Download SDL3 from its official repository
CPMAddPackage(
    NAME SDL3
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG release-3.4.18
    OPTIONS
        "SDL_SHARED ON"
        "SDL_STATIC OFF"
        "SDL_TEST OFF"
        "SDL_TEST_LIBRARY OFF"
)

# B. Download libcurl in a portable way
CPMAddPackage(
    NAME curl
    GIT_REPOSITORY "https://github.com/curl/curl.git"
    GIT_TAG curl-8_23_0
    OPTIONS
        "BUILD_CURL_EXE OFF"
        "BUILD_EXAMPLES OFF"
        "BUILD_LIBCURL_DISABLE_LDAP ON"
        "BUILD_LIBCURL_DOCS OFF"
        "BUILD_MISC_DOCS OFF"
        "BUILD_SHARED_LIBS ON"
        "BUILD_TESTING OFF"
        "CMAKE_DISABLE_FIND_PACKAGE_Libssh2 ON" # Disables SSH2 scanning completely
        "CURL_BROTLI OFF"
        "CURL_DISABLE_COOKIES ON"
        "CURL_USE_LIBIDN2 OFF"
        "CURL_USE_LIBPSL OFF"
        "CURL_USE_OPENSSL OFF"
        "CURL_USE_SCHANNEL OFF"
        "CURL_ZLIB OFF"
        "CURL_ZSTD OFF"
        "ENABLE_ARES OFF"
        "HTTP_ONLY ON"
        "USE_NGHTTP2 OFF"
)

# C. Download Dear ImGui
CPMAddPackage(
    NAME imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG v1.93.0
)

# --- 4. The special case of FFmpeg ---
set(ffmpeg_components avformat avcodec avutil swscale)
if(WIN32)  # Precompiled binaries for Windows
    message(STATUS "Windows environment detected. Configuring native FFmpeg dependencies...")

    set(FFMPEG_ROOT "" CACHE PATH "Root directory of FFmpeg installation")

    find_path(FFMPEG_INCLUDE_DIR
        NAMES libavformat/avformat.h
        HINTS "${FFMPEG_ROOT}/include"
        REQUIRED
    )

    foreach(component IN LISTS ffmpeg_components)
        find_library(FFMPEG_${component}_LIBRARY
            NAMES ${component}
            HINTS "${FFMPEG_ROOT}/lib"
            REQUIRED
        )
        add_library(${component} SHARED IMPORTED)
        set_target_properties(${component} PROPERTIES
            IMPORTED_IMPLIB "${FFMPEG_${component}_LIBRARY}"
            # IMPORTED_LOCATION "${FFMPEG_${component}_DLL}"
        )
        target_include_directories(${component} INTERFACE "${FFMPEG_INCLUDE_DIR}")
        add_library(ffmpeg::${component} ALIAS ${component})
    endforeach()
else()
    message(STATUS "Linux/POSIX environment detected. Using native PkgConfig...")
    # On Linux / Raspberry Pi, the native standard is maintained
    find_package(PkgConfig REQUIRED)
    foreach(component IN LISTS ffmpeg_components)
        pkg_check_modules(${component} REQUIRED IMPORTED_TARGET lib${component})
        add_library(ffmpeg::${component} ALIAS PkgConfig::${component})
    endforeach()
endif()

# --- 5. Add app ---
add_subdirectory(dsc6620g)
