cmake_minimum_required(VERSION 3.16)
project(telemetry_dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try find_package first
find_package(MAVSDK QUIET)

if(NOT MAVSDK_FOUND)
    # Fallback to pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MAVSDK REQUIRED mavsdk)
endif()

find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

# Include directories
if(MAVSDK_FOUND)
    # Using find_package
    include_directories(src)
else()
    # Using pkg-config
    include_directories(${MAVSDK_INCLUDE_DIRS})
    include_directories(src)
endif()

# Source files
set(SOURCES
        src/main.cpp
        src/telemetry_reader.cpp
        src/display_manager.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
if(MAVSDK_FOUND)
    target_link_libraries(${PROJECT_NAME}
            MAVSDK::mavsdk
            ${CURSES_LIBRARIES}
            Threads::Threads
    )
else()
    target_link_libraries(${PROJECT_NAME}
            ${MAVSDK_LIBRARIES}
            ${CURSES_LIBRARIES}
            Threads::Threads
    )
    target_compile_options(${PROJECT_NAME} PRIVATE ${MAVSDK_CFLAGS_OTHER})
endif()