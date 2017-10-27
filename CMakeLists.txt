# ============================================================================
#
# CMake file for PeerApi
#
# ============================================================================

cmake_minimum_required(VERSION 3.2)
project(peerapi)


# ============================================================================
# The version number.
# ============================================================================

set(PACKAGE "peerapi")
set(CPACK_PACKAGE_NAME "${PACKAGE}")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE} ${PACKAGE_VERSION}")

option(PEERAPI_WITH_STATIC "Build the static version of the library" ON)
option(PEERAPI_WITH_SHARED "Build the shared version of the library" ON)
option(PEERAPI_BUILD_EXAMPLE "Build the example application" ON)
option(PEERAPI_BUILD_TEST "Build test application" ON)

if (NOT (PEERAPI_WITH_STATIC OR PEERAPI_WITH_SHARED))
	message(FATAL_ERROR "Makes no sense to compile with neither static nor shared libraries.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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
    "src/peerapi.h"
    "src/common.h"
    "src/control.h"
    "src/controlobserver.h"
    "src/peer.h"
    "src/signalconnection.h"
    "src/fakeaudiocapturemodule.h"
    "src/logging.h"
    )

set(SOURCES
    "src/peerapi.cc"
    "src/control.cc"
    "src/peer.cc"
    "src/signalconnection.cc"
    "src/fakeaudiocapturemodule.cc"
    "src/logging.cc"
    )

# ============================================================================
# Target settings
# ============================================================================

set(_PEERAPI_INTERNAL_DEFINES
    ${WEBRTC_DEFINES}
    ${WEBSOCKETPP_DEFINES}
    )

set(_PEERAPI_INTERNAL_INCLUDE_DIR
    "${WEBRTC_INCLUDE_DIR}"
    "${ASIO_INCLUDE_DIR}"
    "${WEBSOCKETPP_INCLUDE_DIIR}"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    )

set(_PEERAPI_INTERNAL_LIBRARIES
    "${WEBRTC_LIBRARIES_INTERNAL}"
    "${WEBRTC_LIBRARIES_EXTERNAL}"
)

set(PEERAPI_INCLUDE_DIRECTORY
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    )

if (PEERAPI_WITH_STATIC)
  add_library(peerapi STATIC ${HEADERS} ${SOURCES})

  target_compile_definitions(peerapi PRIVATE ${_PEERAPI_INTERNAL_DEFINES})
  target_include_directories(peerapi PRIVATE ${_PEERAPI_INTERNAL_INCLUDE_DIR} )

  if (MSVC)
    add_custom_command(TARGET peerapi PRE_BUILD
      COMMAND echo Building WebRTC library
      COMMAND set PATH=${DEPOT_TOOLS_DIR};%PATH%
      COMMAND cmd.exe /C gn gen ${WEBRTC_OUT_DIR} --args=\"is_debug=false is_component_build=false target_cpu=\\\"x86\\\"\"
      COMMAND cmd.exe /C gn gen ${WEBRTC_OUT_DIR_DEBUG} --args=\"is_debug=true is_component_build=false target_cpu=\\\"x86\\\"\"
      COMMAND ninja -C ${WEBRTC_OUT_DIR} peerconnection_client
      COMMAND ninja -C ${WEBRTC_OUT_DIR_DEBUG} peerconnection_client
      WORKING_DIRECTORY "${WEBRTC_ROOT_DIR}"
      COMMENT "Building WebRTC library (Once)"
    )
  elseif (APPLE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
    if (_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
      add_custom_command(TARGET peerapi PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E env \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\" gn gen ${WEBRTC_OUT_DIR_DEBUG} --args=\"is_debug=true is_component_build=false\"
        COMMAND ${CMAKE_COMMAND} -E env \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\" ninja -C ${WEBRTC_OUT_DIR_DEBUG}
        WORKING_DIRECTORY "${WEBRTC_ROOT_DIR}"
        COMMENT "Building WebRTC library (Once)"
        )
    else()
      add_custom_command(TARGET peerapi PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E env \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\" gn gen ${WEBRTC_OUT_DIR} --args=\"is_debug=false is_component_build=false\"
        COMMAND ${CMAKE_COMMAND} -E env \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\" ninja -C ${WEBRTC_OUT_DIR}
        WORKING_DIRECTORY "${WEBRTC_ROOT_DIR}"
        COMMENT "Building WebRTC library (Once)"
        )
    endif()
  elseif(UNIX)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
    if (_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
      add_custom_command(TARGET peerapi PRE_LINK
        COMMAND export \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\"
        COMMAND gn gen ${WEBRTC_OUT_DIR_DEBUG} --args=\"is_debug=true is_component_build=false\"
        COMMAND ninja -C ${WEBRTC_OUT_DIR_DEBUG} peerconnection_client
        WORKING_DIRECTORY "${WEBRTC_ROOT_DIR}"
        COMMENT "Building WebRTC library (Once)"
        )
    else()
      add_custom_command(TARGET peerapi PRE_LINK
        COMMAND export \"PATH=${DEPOT_TOOLS_DIR}:$ENV{PATH}\"
        COMMAND gn gen ${WEBRTC_OUT_DIR} --args=\"is_debug=false is_component_build=false\"
        COMMAND ninja -C ${WEBRTC_OUT_DIR} peerconnection_client
        WORKING_DIRECTORY "${WEBRTC_ROOT_DIR}"
        COMMENT "Building WebRTC library (Once)"
        )
    endif()

  endif(MSVC)

  if (WIN32)
    # Windows uses the same .lib ending for static libraries and shared
    # library linker files, so rename the static library.
    set_target_properties(peerapi PROPERTIES OUTPUT_NAME peerapi_static)
  endif()

  if (MSVC)
    MERGE_STATIC_LIBRARIES( peerapi
                            "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                            "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}"
                            "${WEBRTC_OBJECTS_INTERNAL_RELEASE}"
                            "${WEBRTC_OBJECTS_INTERNAL_DEBUG}"
                            )
  elseif (APPLE)
    MERGE_STATIC_LIBRARIES(peerapi
                           "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                           "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}"
                           "${WEBRTC_OBJECTS_INTERNAL_RELEASE}"
                           "${WEBRTC_OBJECTS_INTERNAL_DEBUG}"
                           )
  elseif (UNIX)
    MERGE_STATIC_LIBRARIES(peerapi
                           "${WEBRTC_LIBRARIES_INTERNAL_RELEASE}"
                           "${WEBRTC_LIBRARIES_INTERNAL_DEBUG}"
                           "${WEBRTC_OBJECTS_INTERNAL_RELEASE}"
                           "${WEBRTC_OBJECTS_INTERNAL_DEBUG}"
                           )
  endif(MSVC)

endif()

if (PEERAPI_WITH_SHARED)
  add_library(peerapi_shared SHARED ${HEADERS} ${SOURCES})

  target_compile_definitions(peerapi_shared PRIVATE ${_PEERAPI_INTERNAL_DEFINES})
  target_include_directories(peerapi_shared PRIVATE ${_PEERAPI_INTERNAL_INCLUDE_DIR} )
  set_target_properties(peerapi_shared PROPERTIES OUTPUT_NAME peerapi)
  target_link_libraries(peerapi_shared ${_PEERAPI_INTERNAL_LIBRARIES} ${WEBRTC_OBJECTS_INTERNAL})

  if (WIN32)
#  # Compile as DLL (export function declarations)
#    set_property(
#      TARGET peerapi_shared
#      PROPERTY COMPILE_DEFINITIONS)
  endif()

  if (APPLE)
    set_property(TARGET peerapi_shared PROPERTY MACOSX_RPATH YES)
  endif()

endif()


# ============================================================================
# Variables for parent project including peerapi using add_subdirectory()
# ============================================================================

set(PEERAPI_INCLUDE_DIR ${PEERAPI_INCLUDE_DIRECTORY} 
                                 CACHE STRING "PeerApi include directories")
if (PEERAPI_WITH_STATIC)
  set(PEERAPI_LIBRARIES_STATIC peerapi ${WEBRTC_LIBRARIES_EXTERNAL}
                                  CACHE STRING "PeerApi static library")
endif()
if (PEERAPI_WITH_SHARED)
  set(PEERAPI_LIBRARIES_SHARED peerapi_shared ${WEBRTC_LIBRARIES_EXTERNAL}
                                  CACHE STRING "PeerApi shared library")
endif()

# ============================================================================
# Test
# ============================================================================

if (PEERAPI_BUILD_TEST)
  add_executable(test_main src/test/test_main.cc)
  add_dependencies(test_main peerapi)
  target_include_directories(test_main PRIVATE ${PEERAPI_INCLUDE_DIR})
  target_link_libraries(test_main ${PEERAPI_LIBRARIES_STATIC})
  set_target_properties (test_main PROPERTIES FOLDER test)

  add_test(test_main test_main)
endif(PEERAPI_BUILD_TEST)

# ============================================================================
# Example
# ============================================================================

if (PEERAPI_BUILD_EXAMPLE)
  # echo server
  add_executable(echo_server examples/echo_server/main.cc)
  add_dependencies(echo_server peerapi)
  target_include_directories(echo_server PRIVATE ${PEERAPI_INCLUDE_DIR})
  target_link_libraries(echo_server ${PEERAPI_LIBRARIES_STATIC})
  set_target_properties (echo_server PROPERTIES FOLDER examples)

  # echo client
  add_executable(echo_client examples/echo_client/main.cc)
  add_dependencies(echo_client peerapi)
  target_include_directories(echo_client PRIVATE ${PEERAPI_INCLUDE_DIR})
  target_link_libraries(echo_client ${PEERAPI_LIBRARIES_STATIC})
  set_target_properties (echo_client PROPERTIES FOLDER examples)

  # p2p netcat
  add_executable(p2p_netcat examples/p2p_netcat/main.cc)
  add_dependencies(p2p_netcat peerapi)
  target_include_directories(p2p_netcat PRIVATE ${PEERAPI_INCLUDE_DIR})
  target_link_libraries(p2p_netcat ${PEERAPI_LIBRARIES_STATIC})
  set_target_properties (p2p_netcat PROPERTIES FOLDER examples)
  set_target_properties (p2p_netcat PROPERTIES OUTPUT_NAME pnc)
endif (PEERAPI_BUILD_EXAMPLE)
