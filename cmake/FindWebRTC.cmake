# The following variables will be defined:
#
#  WEBRTC_FOUND
#  WEBRTC_DEFINES
#  WEBRTC_INCLUDE_DIR
#  WEBRTC_LIBRARIES
#

# ============================================================================
# WebRTC root and default library directory
# ============================================================================

set(WEBRTC_ROOT_DIR
  ${PROJECT_SOURCE_DIR}/libs/webrtc/src
  CACHE PATH
  "WebRTC root directory."
  )

set(WEBRTC_LIBRARY_DIR
  ${PROJECT_SOURCE_DIR}/out/Release
  CACHE PATH
  "WebRTC output directory"
  )

# ============================================================================
# Find WebRTC header directory
# ============================================================================

find_path(WEBRTC_INCLUDE_DIR
  NAMES
  	webrtc/config.h
  PATHS
  	${WEBRTC_ROOT_DIR}
  )


# ============================================================================
# Find WebRTC libries
#   webrtc -> webrtc.lib or libwebrtc.a
#   libyuv yuv -> libyuv.lib or libyuv.a
#   ...
# ============================================================================

set (WEBRTC_LIBRARIES)

list(APPEND _WEBRTC_LIB_NAMES
  "webrtc"
  "libyuv yuv"
  )

foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)
  find_library(_WEBRTC_LIB_PATH
    NAMES ${lib}
    PATHS 
      ${WEBRTC_LIBRARY_DIR}
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc
    )        
  if (_WEBRTC_LIB_PATH)
    list(APPEND
      WEBRTC_LIBRARIES
      ${_WEBRTC_LIB_PATH}
      )
  else(_WEBRTC_LIB_PATH)
    message(FATAL_ERROR "WebRTC module '${lib}' was not found.\n"
        "Please check 'WEBRTC_ROOT_DIR' and 'WEBRTC_LIBRARY_DIR'.\n")
  endif()
endforeach()
  
if(WIN32 AND MSVC)
  list(APPEND
    WEBRTC_LIBRARIES
      Secur32.lib Winmm.lib msdmo.lib dmoguids.lib wmcodecdspuuid.lib
      wininet.lib dnsapi.lib version.lib ws2_32.lib Strmiids.lib
    )    
endif()

if(UNIX)
  find_package (Threads REQUIRED)
  if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(CORE_SERVICES_LIBRARY CoreServices)
  endif()
  list(APPEND WEBRTC_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${FOUNDATION_LIBRARY}
    ${CORE_FOUNDATION_LIBRARY}
    ${CORE_SERVICES_LIBRARY}
    )
endif()


# ============================================================================
# Definitions
# ============================================================================

set(WEBRTC_DEFINES)

if (MSVC)
  list(APPEND WEBRTC_DEFINES)
endif(MSVC)


# ============================================================================
# Validation
# ============================================================================

if (WEBRTC_INCLUDE_DIR AND _WEBRTC_LIB_PATH)
  set(WEBRTC_FOUND 1)
else()
  set(WEBRTC_FOUND 0)
endif()

if(NOT WEBRTC_FOUND)
  message(FATAL_ERROR "WebRTC file was not found.\n"
      "Please check 'WEBRTC_ROOT_DIR' and 'WEBRTC_LIBRARY_DIR'.\n")
endif()
