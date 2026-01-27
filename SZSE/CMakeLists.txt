cmake_minimum_required(VERSION 3.31.6)
project(SZSE)

set(CMAKE_CXX_STANDARD 11)
#-std=c++11 instead of gnu++11
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(../3rd_lib/include ${PROJECT_SOURCE_DIR})

#file(READ "/proc/version" PROC_VERSION)
#if(PROC_VERSION MATCHES ".*[Mm]icrosoft.*|.*WSL.*")
#    message(STATUS "Detected WSL environment")
#    link_directories(../3rd_lib/lib_wsl)
#else()
#    message(STATUS "Detected standard Linux environment")
#    link_directories(../3rd_lib/lib)
#endif()

add_executable(SZSE main.cpp
        myInih.cpp
        session.cpp
        utility.cpp
        loggerSingleton.cpp
        szMdParser.cpp
        threadFuncs.cpp
        myFIFOQueue.cpp
        business.cpp
        shmStorage.cpp
)

#target_link_libraries(SZSE spdlog pthread)
target_link_libraries(SZSE pthread rt)

add_executable(logTest ut/logTest.cpp loggerSingleton.cpp)
target_link_libraries(logTest pthread)

add_executable(queueTest ut/queueSingleProduceConsumerTest.cpp myFIFOQueue.cpp)
target_link_libraries(queueTest pthread)

add_executable(shmStorageTest ut/shmStorageTest.cpp)
target_link_libraries(shmStorageTest pthread rt)

add_executable(queueNLTest ut/queueNLTest.cpp myFIFOQueue.cpp myFIFOQueueNL.cpp)
target_link_libraries(queueNLTest pthread)
