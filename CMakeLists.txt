# cmake_minimum_required(VERSION 2.8)start
# PROJECT(test_preprocess)
# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
# ADD_DEFINITIONS(-DWEBRTC_POSIX)

# AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

# include_directories(WebRtcMoudle/.)
# include_directories(.)

# add_executable(test_resample 
# test_resample.cc
# ${WebRtcMoudle}
# preprocess.cc
# resampler.cc
# )
# TARGET_LINK_LIBRARIES(test_resample pthread)

cmake_minimum_required(VERSION 2.8)
PROJECT(test_resample)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

include_directories(WebRtcMoudle/.)
include_directories(.)

add_executable(test_resample 
test_resample.cc
${WebRtcMoudle}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_resample pthread)



cmake_minimum_required(VERSION 2.8)
PROJECT(test_no_sample_t)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

include_directories(WebRtcMoudle/.)
include_directories(.)

add_executable(test_no_sample_t 
test_no_sample_t.cc
${WebRtcMoudle}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample_t pthread)



PROJECT(test_no_sample_960)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

include_directories(WebRtcMoudle/.)
include_directories(.)

add_executable(test_no_sample_960 
test_no_sample_960.cc
${WebRtcMoudle}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample_960 pthread)





PROJECT(test_no_sample_1024*4)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

include_directories(WebRtcMoudle/.)
include_directories(.)

add_executable(test_no_sample_1024*4 
test_no_sample_1024*4.cc
${WebRtcMoudle}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample_1024*4 pthread)




PROJECT(test_no_sample_1024*4_give_up)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb ")
ADD_DEFINITIONS(-DWEBRTC_POSIX)
AUX_SOURCE_DIRECTORY(WebRtcMoudle/. WebRtcMoudle)

include_directories(WebRtcMoudle/.)
include_directories(.)

add_executable(test_no_sample_1024*4_give_up 
test_no_sample_1024*4_give_up.cc
${WebRtcMoudle}
preprocess.cc
resampler.cc
)
TARGET_LINK_LIBRARIES(test_no_sample_1024*4_give_up pthread)
