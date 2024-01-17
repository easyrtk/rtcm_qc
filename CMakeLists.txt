cmake_minimum_required(VERSION 3.16)
project(rtcm_qc)

set(CMAKE_CXX_STANDARD 17)

include_directories(./)

add_executable(rtcm_qc
    rtcm_qc.cpp
    rtcm_buff.c
    rtcm_buff.h
    )

#find_package(Threads REQUIRED)
target_link_libraries(rtcm_qc)