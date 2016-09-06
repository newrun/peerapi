# ============================================================================
#
# CMake file for PeerConnect
#
# ============================================================================

cmake_minimum_required(VERSION 2.8)
project(peerconnect)


# ============================================================================
# The version number.
# ============================================================================

set(PACKAGE "peerconnect")
set(CPACK_PACKAGE_NAME "${PACKAGE}")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE} ${PACKAGE_VERSION}")

option(PC_WITH_STATIC "Build the static version of the library" ON)
option(PC_WITH_SHARED "Build the shared version of the library" ON)

if (NOT (PC_WITH_STATIC OR PC_WITH_SHARED))
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
elseif (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fvisibility=hidden")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
elseif (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


# ============================================================================
# Find external dependencies
# ============================================================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
include(Mergestaticlibs)

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
    "src/peerconnect.h"
    "src/common.h"
    "src/control.h"
    "src/controlobserver.h"
    "src/peer.h"
    "src/signalconnection.h"
    "src/fakeaudiocapturemodule.h"
    "src/logging.h"
    )

set(SOURCES
    "src/peerconnect.cc"
    "src/control.cc"
    "src/peer.cc"
    "src/signalconnection.cc"
    "src/fakeaudiocapturemodule.cc"
    "src/logging.cc"
    )

# ============================================================================
# Target settings
# ============================================================================

set(_PC_INTERNAL_DEFINES
    ${WEBRTC_DEFINES}
    ${WEBSOCKETPP_DEFINES}
    )

set(_PC_INTERNAL_INCLUDE_DIR
    "${WEBRTC_INCLUDE_DIR}"
    "${ASIO_INCLUDE_DIR}"
    "${WEBSOCKETPP_INCLUDE_DIIR}"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    )

set(_PC_INTERNAL_LIBRARIES
    "${WEBRTC_LIBRARIES_INTERNAL}"
    "${WEBRTC_LIBRARIES_EXTERNAL}"
)

set(PC_INCLUDE_DIRECTORY
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    )

if (PC_WITH_STATIC)
  add_library(peerconnect STATIC ${HEADERS} ${SOURCES})

  target_compile_definitions(peerconnect PRIVATE ${_PC_INTERNAL_DEFINES})
  target_include_directories(peerconnect PRIVATE ${_PC_INTERNAL_INCLUDE_DIR} )

  if (WIN32)
    # Windows uses the same .lib ending for static libraries and shared
    # library linker files, so rename the static library.
    set_target_properties(peerconnect PROPERTIES OUTPUT_NAME peerconnect_static)
  endif()

  if (MSVC)
    MERGE_STATIC_LIBRARIES( peerconnect
                            "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                            "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}")
  elseif (APPLE)
    MERGE_STATIC_LIBRARIES(peerconnect
                           "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                           "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}")
  elseif (UNIX)
    MERGE_STATIC_LIBRARIES(peerconnect
                           "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                           "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}")
  endif(MSVC)
endif()

if (PC_WITH_SHARED)
  add_library(peerconnect_shared SHARED ${HEADERS} ${SOURCES})

  target_compile_definitions(peerconnect_shared PRIVATE ${_PC_INTERNAL_DEFINES})
  target_include_directories(peerconnect_shared PRIVATE ${_PC_INTERNAL_INCLUDE_DIR} )

  set_target_properties(peerconnect_shared PROPERTIES OUTPUT_NAME peerconnect)
  target_link_libraries(peerconnect_shared ${_PC_INTERNAL_LIBRARIES})

  if (WIN32)
#  # Compile as DLL (export function declarations)
#    set_property(
#      TARGET peerconnect_shared
#      PROPERTY COMPILE_DEFINITIONS)
  endif()

  if (APPLE)
    set_property(TARGET peerconnect_shared PROPERTY MACOSX_RPATH YES)
  endif()

endif()


# ============================================================================
# Variables for parent project including peerconnect using add_subdirectory()
# ============================================================================

set(PEERCONNECT_INCLUDE_DIRECTORY ${PC_INCLUDE_DIRECTORY} 
                                 CACHE STRING "PeerConnect include directories")
if (PC_WITH_STATIC)
  set(PEERCONNECT_LIBRARIES_STATIC peerconnect ${WEBRTC_LIBRARIES_EXTERNAL}
                                  CACHE STRING "PeerConnect static library")
endif()
if (PC_WITH_SHARED)
  set(PEERCONNECT_LIBRARIES_SHARED peerconnect_shared ${WEBRTC_LIBRARIES_EXTERNAL}
                                  CACHE STRING "PeerConnect shared library")
endif()


# ============================================================================
# Test
# ============================================================================

add_executable(test_main src/test/test_main.cc)
add_dependencies(test_main peerconnect)

target_include_directories(test_main PRIVATE ${PEERCONNECT_INCLUDE_DIRECTORY})
target_link_libraries(test_main ${PEERCONNECT_LIBRARIES_STATIC})
set_target_properties (test_main PROPERTIES FOLDER test)

add_test(test_main test_main)


# ============================================================================
# Example
# ============================================================================

# echo server
add_executable(echo_server examples/echo_server/main.cc)
add_dependencies(echo_server peerconnect)
target_include_directories(echo_server PRIVATE ${PEERCONNECT_INCLUDE_DIRECTORY})
target_link_libraries(echo_server ${PEERCONNECT_LIBRARIES_STATIC})
set_target_properties (echo_server PROPERTIES FOLDER examples)

# echo client
add_executable(echo_client examples/echo_client/main.cc)
add_dependencies(echo_client peerconnect)
target_include_directories(echo_client PRIVATE ${PEERCONNECT_INCLUDE_DIRECTORY})
target_link_libraries(echo_client ${PEERCONNECT_LIBRARIES_STATIC})
set_target_properties (echo_client PROPERTIES FOLDER examples)

# p2p netcat
add_executable(p2p_netcat examples/p2p_netcat/main.cc)
add_dependencies(p2p_netcat peerconnect)
target_include_directories(p2p_netcat PRIVATE ${PEERCONNECT_INCLUDE_DIRECTORY})
target_link_libraries(p2p_netcat ${PEERCONNECT_LIBRARIES_STATIC})
set_target_properties (p2p_netcat PROPERTIES FOLDER examples)
set_target_properties (p2p_netcat PROPERTIES OUTPUT_NAME pnc)
