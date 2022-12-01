cmake_minimum_required(VERSION 2.8)
PROJECT(webRTC)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} main_srs)

include_directories(${PROJECT_SOURCE_DIR}/WebRtcModule)
include_directories(${PROJECT_SOURCE_DIR})

add_executable(webRTCtest  ${main_srs})

TARGET_LINK_LIBRARIES(webRTCtest ${PROJECT_SOURCE_DIR}/WebRtcMoudle/build/libwebRTC.so)
TARGET_LINK_LIBRARIES(webRTCtest libpthread.so)
