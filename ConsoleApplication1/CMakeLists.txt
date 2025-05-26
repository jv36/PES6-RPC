cmake_minimum_required(VERSION 3.10)
project(ConsoleApplication1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Force 32-bit compilation
set(CMAKE_GENERATOR_PLATFORM Win32)
set(CMAKE_VS_PLATFORM_NAME Win32)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Add Windows-specific compile definitions
add_definitions(-D_WIN32_WINNT=0x0601)  # Target Windows 7 or later
add_definitions(-DWINVER=0x0601)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Add include directories
include_directories(${ATL_INCLUDE_DIRS})
include_directories("$ENV{VCINSTALLDIR}Tools/MSVC/$ENV{VCToolsVersion}/atlmfc/include")

# Add the executable
add_executable(ConsoleApplication1 main.cpp)

# Link against Windows libraries
link_directories("$ENV{VCINSTALLDIR}Tools/MSVC/$ENV{VCToolsVersion}/atlmfc/lib/x86")
target_link_libraries(ConsoleApplication1 PRIVATE
    kernel32
    user32
    atls
)
