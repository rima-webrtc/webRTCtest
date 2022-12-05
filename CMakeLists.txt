cmake_minimum_required(VERSION 2.8)
PROJECT(test_preprocess)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)

AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcModule)

include_directories(WebRtcModule/.)
include_directories(.)

add_executable(test_preprocess 
test_preprocess.cc
${WebRtcModule}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_preprocess pthread)

cmake_minimum_required(VERSION 2.8)


PROJECT(test_no_sample)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcModule/. WebRtcModule)

include_directories(WebRtcModule/.)
include_directories(.)

add_executable(test_no_sample 
test_no_sample.cc
${WebRtcModule}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample pthread)




PROJECT(test_no_sample_t)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcModule/. WebRtcModule)

include_directories(WebRtcModule/.)
include_directories(.)

add_executable(test_no_sample_t 
test_no_sample_t.cc
${WebRtcModule}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample_t pthread)
