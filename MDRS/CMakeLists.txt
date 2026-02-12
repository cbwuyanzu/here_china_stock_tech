cmake_minimum_required(VERSION 3.31.6)
project(SZSE)

set(CMAKE_CXX_STANDARD 11)
#-std=c++11 instead of gnu++11
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

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
        myPub/myInih.cpp
        session.cpp
        myPub/utility.cpp
        myPub/loggerSingleton.cpp
        XSHEMdParser.cpp
        XSHEThreadFuncs.cpp
        myPub/myFIFOQueue.cpp
        XSHEBusiness.cpp
        myPub/shmStorage.cpp
        appManager.cpp
        appManager.h
)

target_link_libraries(SZSE pthread rt)

add_executable(logTest ut/logTest.cpp myPub/loggerSingleton.cpp)
target_link_libraries(logTest pthread)

add_executable(queueTest ut/queueSingleProduceConsumerTest.cpp myPub/myFIFOQueue.cpp)
target_link_libraries(queueTest pthread)

add_executable(shmStorageTest ut/shmStorageTest.cpp)
target_link_libraries(shmStorageTest pthread rt)

add_executable(queueNLTest ut/queueNLTest.cpp myPub/myFIFOQueue.cpp myPub/myFIFOQueueNL.cpp)
target_link_libraries(queueNLTest pthread)
