# Install script for directory: /home/fengmao/cowa/webRTCtest/WebRtcMoudle

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/build/libwebRTC.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libwebRTC.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/build/libwebRTC.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/webRtcModule" TYPE FILE FILES
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/analog_agc.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/complex_fft_tables.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/cpu_features_wrapper.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/defines.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/digital_agc.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/fft4g.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/gain_control.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/noise_suppression.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/noise_suppression_x.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/ns_core.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/nsx_core.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/nsx_defines.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/real_fft.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/resample_by_2_internal.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/ring_buffer.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/signal_processing_library.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/spl_inl.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/typedefs.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/windows_private.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/aec_core.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/aec_core_internal.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/aec_rdft.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/aec_resampler.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/echo_cancellation.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/echo_cancellation_internal.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/delay_estimator.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/delay_estimator_internal.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/delay_estimator_wrapper.h"
    "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/ring_buffer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/fengmao/cowa/webRTCtest/WebRtcMoudle/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
