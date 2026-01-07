cmake_minimum_required(VERSION 3.31.6)
project(SZSE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(../3rd_lib/include)

file(READ "/proc/version" PROC_VERSION)
if(PROC_VERSION MATCHES ".*[Mm]icrosoft.*|.*WSL.*")
    message(STATUS "Detected WSL environment")
    link_directories(../3rd_lib/lib_wsl)
else()
    message(STATUS "Detected standard Linux environment")
    link_directories(../3rd_lib/lib)
endif()

add_executable(SZSE main.cpp
        configuration.cpp
        session.cpp
        utility.cpp
        LoggerSingleton.cpp)

target_link_libraries(SZSE spdlog pthread)
