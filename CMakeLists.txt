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

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

# Websocketpp
find_package(Websocketpp)

# standalone asio for websocketpp
find_package(Asio)



# ============================================================================
# Headers and sources.
# ============================================================================
set(HEADERS
  "src/throughnet.h"
  "src/control.h"
  "src/controlobserver.h"
  "src/peer.h"
  "src/signalconnection.h"
  "src/fakeaudiocapturemodule.h"
  )

set(SOURCES
  "src/throughnet.cc"
  "src/control.cc"
  "src/peer.cc"
  "src/signalconnection.cc"
  "src/fakeaudiocapturemodule.cc"
  )

# ============================================================================
# Target settings
# ============================================================================

set(_TN_INTERNAL_INCLUDE_DIR
  "${WEBRTC_INCLUDE_DIR}"
  "${ASIO_INCLUDE_DIR}"
  "${WEBSOCKETPP_INCLUDE_DIIR}"
  "${WEBRTC_BORING_SSL_INCLUDE}"
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/src"
  )

set(_TN_INTERNAL_LIBRARIES
  "${WEBRTC_LIBRARIES}"
)

set(TN_INCLUDE_DIRECTORY
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/src"
  )

if (TN_WITH_STATIC)
  add_library(throughnet STATIC
    ${HEADERS}
    ${SOURCES})
  target_compile_definitions(throughnet PRIVATE ${WEBRTC_DEFINES} ${WEBSOCKETPP_DEFINES})
  target_include_directories(throughnet PRIVATE ${_TN_INTERNAL_INCLUDE_DIR} )

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
  target_compile_definitions(throughnet_shared PRIVATE ${WEBRTC_DEFINES} ${WEBSOCKETPP_DEFINES})
  target_include_directories(throughnet_shared PRIVATE ${_TN_INTERNAL_INCLUDE_DIR} )

  set_target_properties(throughnet_shared
    PROPERTIES
    OUTPUT_NAME throughnet
    )

  target_link_libraries(throughnet_shared ${_TN_INTERNAL_LIBRARIES})

  if (WIN32)
#  # Compile as DLL (export function declarations)
#    set_property(
#      TARGET throughnet_shared
#      PROPERTY COMPILE_DEFINITIONS)
  endif()

  if (APPLE)
    set_property(TARGET throughnet_shared PROPERTY MACOSX_RPATH YES)
  endif()

endif()



# ============================================================================
# Target dependencies
# ============================================================================


# ============================================================================
# Variables for parent project including throughnet using add_subdirectory()
# ============================================================================

set(THROUGHNET_INCLUDE_DIRECTORY ${TN_INCLUDE_DIRECTORY} CACHE STRING "ThroughNet include directories")
if (TN_WITH_STATIC)
  set(THROUGHNET_LIBRARIES_STATIC throughnet ${WEBRTC_LIBRARIES} CACHE STRING "ThroughNet static library")
endif()
if (TN_WITH_SHARED)
  set(THROUGHNET_LIBRARIES_SHARED throughnet_shared CACHE STRING "ThroughNet shared library")
endif()


# ============================================================================
# Test
# ============================================================================

add_executable(test_main src/test/test_main.cc)
add_dependencies(test_main throughnet)

target_compile_definitions(test_main PRIVATE ${WEBRTC_DEFINES} ${WEBSOCKETPP_DEFINES})
target_include_directories(test_main PRIVATE ${_TN_INTERNAL_INCLUDE_DIR})
target_link_libraries(test_main throughnet ${_TN_INTERNAL_LIBRARIES})
set_target_properties (test_main PROPERTIES FOLDER test)

add_test(test_main test_main)


# ============================================================================
# Example
# ============================================================================

# echo server
add_executable(echo_server examples/echo_server/main.cc)
add_dependencies(echo_server throughnet)

target_include_directories(echo_server PRIVATE ${THROUGHNET_INCLUDE_DIRECTORY})
target_link_libraries(echo_server throughnet ${THROUGHNET_LIBRARIES_STATIC})
set_target_properties (echo_server PROPERTIES FOLDER examples)

# echo client
add_executable(echo_client examples/echo_client/main.cc)
add_dependencies(echo_client throughnet)

target_include_directories(echo_client PRIVATE ${THROUGHNET_INCLUDE_DIRECTORY})
target_link_libraries(echo_client throughnet ${THROUGHNET_LIBRARIES_STATIC})
set_target_properties (echo_client PROPERTIES FOLDER examples)
