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
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
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
  set(WEBRTC_POSIX_OR_WIN WEBRTC_POSIX)
elseif (WIN32)
  set(WEBRTC_POSIX_OR_WIN WEBRTC_WIN)
endif()

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(${WEBRTC_DEFINES})


# ============================================================================
# Find external dependencies
# ============================================================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# WebRTC
find_package(WebRTC)

# zlib
set(SKIP_INSTALL_ALL ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/libs/zlib)
set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/libs/zlib)
if (MSVC)
list(APPEND
  ZLIB_LIBRARIES
  optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstatic.lib
  debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstaticd.lib
  )
else()
list(APPEND
  ZLIB_LIBRARIES
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib.a
  )
endif()

# libwebsockets
set(LWS_WITH_STATIC ON)
set(LWS_WITH_SHARED OFF)
set(LWS_WITHOUT_CLIENT OFF)
set(LWS_WITHOUT_SERVER ON)
set(LWS_WITH_SSL ON)
set(LWS_WITH_ZLIB ON)
set(LWS_USE_BUNDLED_ZLIB OFF)
set(LWS_WITHOUT_TESTAPPS ON)
set(LWS_WITHOUT_TEST_CLIENT ON)
set(LWS_WITHOUT_TEST_ECHO ON)
set(LWS_WITHOUT_TEST_FRAGGLE ON)
set(LWS_WITHOUT_TEST_PING ON)
set(LWS_WITHOUT_TEST_SERVER ON)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON)
set(LWS_ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} CACHE PATH "Path to the zlib include directory")
set(LWS_ZLIB_LIBRARIES ${ZLIB_LIBRARIES} CACHE PATH "Path to the zlib library")
set(LWS_OPENSSL_LIBRARIES ${WEBRTC_BORING_SSL_LIBRARIES} CACHE PATH "Path to the OpenSSL library")
set(LWS_OPENSSL_INCLUDE_DIRS ${WEBRTC_BORING_SSL_INCLUDE} CACHE PATH "Path to the OpenSSL include directory")
set(LWS_WITHOUT_EXTENSIONS ON)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/libs/libwebsockets)


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
  )

set(SOURCES
  "src/throughnet.cc"
  )

# ============================================================================
# Target settings
# ============================================================================

include_directories(
  "${PROJECT_BINARY_DIR}"
  "${WEBRTC_INCLUDE_DIR}"
  "${WEBRTC_INCLUDE_DIR}/webrtc"
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

foreach (lib ${TN_LIBRARIES})
  target_link_libraries(${lib} ${LIB_LIST})
endforeach()


# These will be available to parent projects including throughnet using add_subdirectory()
set(THROUGHNET_LIBRARIES ${TN_LIBRARIES} CACHE STRING "ThroughNet libraries")
if (TN_WITH_STATIC)
  set(THROUGHNET_LIBRARIES_STATIC throughnet CACHE STRING "ThroughNet static library")
endif()
if (TN_WITH_SHARED)
  set(THROUGHNET_LIBRARIES_SHARED throughnet_shared CACHE STRING "ThroughNet shared library")
endif()
