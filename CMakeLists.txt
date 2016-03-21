# ============================================================================
#
# CMake file for ThroughNet
#
# ============================================================================

cmake_minimum_required(VERSION 2.8)
project(throughnet)


# ============================================================================
# The version number.
# ============================================================================

set(PACKAGE "throughnet")
set(CPACK_PACKAGE_NAME "${PACKAGE}")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE} ${PACKAGE_VERSION}")

option(TN_WITH_STATIC "Build the static version of the library" ON)
option(TN_WITH_SHARED "Build the shared version of the library" ON)

if (NOT (TN_WITH_STATIC OR TN_WITH_SHARED))
	message(FATAL_ERROR "Makes no sense to compile with neither static nor shared libraries.")
endif()


# ============================================================================
# The output directory.
# ============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

message(STATUS "CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'")


# ============================================================================
# Platform or compiler specific configurations
# ============================================================================
if (MSVC)
  # Use /MT or /MTd instead of /MD or /MDd
  foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
endif()

if (UNIX)
  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
endif()


# ============================================================================
# Find external dependencies
# ============================================================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# WebRTC
find_package(WebRTC)

# zlib
find_package(ZLIB)

# libwebsockets
find_package(Libwebsockets)

add_definitions(${WEBRTC_DEFINES})

# ============================================================================
# Generate the config.h that includes all the compilation settings.
# ============================================================================
configure_file(
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h")


# ============================================================================
# Headers and sources.
# ============================================================================
set(HEADERS
  "${PROJECT_BINARY_DIR}/config.h"
  "src/throughnet.h"
  "src/control.h"
  "src/peer.h"
  "src/signal.h"
  "src/dummysignal.h"
  "src/fakeaudiocapturemodule.h"
  )

set(SOURCES
  "src/throughnet.cc"
  "src/control.cc"
  "src/peer.cc"
  "src/signal.cc"
  "src/dummysignal.cc"
  "src/fakeaudiocapturemodule.cc"
  )

# ============================================================================
# Target settings
# ============================================================================

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/src"
  "${WEBRTC_INCLUDE_DIR}"
  "${WEBRTC_INCLUDE_DIR}/webrtc"
  "${WEBRTC_INCLUDE_DIR}/third_party"
  "${WEBRTC_INCLUDE_DIR}/third_party/webrtc"
  "${WEBRTC_INCLUDE_DIR}/third_party/wtl/include"
  "${WEBRTC_INCLUDE_DIR}/third_party/jsoncpp/overrides/include"
  "${WEBRTC_INCLUDE_DIR}/third_party/jsoncpp/source/include"
  )

set(TN_LIBRARIES)

if (TN_WITH_STATIC)
  add_library(throughnet STATIC
    ${HEADERS}
    ${SOURCES})
  list(APPEND TN_LIBRARIES throughnet)

  if (WIN32)
    # Windows uses the same .lib ending for static libraries and shared
    # library linker files, so rename the static library.
    set_target_properties(throughnet
      PROPERTIES
      OUTPUT_NAME throughnet_static)
  endif()
endif()

if (TN_WITH_SHARED)
  add_library(throughnet_shared SHARED
    ${HEADERS}
    ${SOURCES})
  list(APPEND TN_LIBRARIES throughnet_shared)

  set_target_properties(throughnet_shared
    PROPERTIES
    OUTPUT_NAME throughnet
    )

  target_link_libraries(throughnet_shared ${WEBRTC_LIBRARIES})

  if (WIN32)
  # Compile as DLL (export function declarations)
    set_property(
      TARGET throughnet_shared
      PROPERTY COMPILE_DEFINITIONS)
  endif()

  if (APPLE)
    set_property(TARGET throughnet_shared PROPERTY MACOSX_RPATH YES)
  endif()
endif()



# ============================================================================
# Target dependencies
# ============================================================================

add_dependencies(websockets zlibstatic)
add_dependencies(websockets zlib)
add_dependencies(throughnet websockets)
add_dependencies(throughnet_shared websockets)

# ============================================================================
# Variables for parent project including throughnet using add_subdirectory()
# ============================================================================

set(THROUGHNET_LIBRARIES ${TN_LIBRARIES} CACHE STRING "ThroughNet libraries")
if (TN_WITH_STATIC)
  set(THROUGHNET_LIBRARIES_STATIC throughnet CACHE STRING "ThroughNet static library")
endif()
if (TN_WITH_SHARED)
  set(THROUGHNET_LIBRARIES_SHARED throughnet_shared CACHE STRING "ThroughNet shared library")
endif()


# ============================================================================
# Test example
# ============================================================================

add_executable(test_main
  src/test/test_main.cc
  )

target_link_libraries(test_main
  throughnet
  ${WEBRTC_LIBRARIES}
  )

add_test(test_main test_main)
