# The following variables will be defined:
#
#  WEBRTC_FOUND
#  WEBRTC_DEFINES
#  WEBRTC_INCLUDE_DIR
#  WEBRTC_LIBRARIES
#
#  WEBRTC_BORING_SSL_INCLUDE
#  WEBRTC_BORING_SSL_LIBRARIES
#

# ============================================================================
# WebRTC root and default library directory
# ============================================================================

if (DEFINED ENV{WEBRTC_ROOT_DIR})
  set(WEBRTC_ROOT_DIR $ENV{WEBRTC_ROOT_DIR})
  message("WEBRTC_ROOT_DIR = '${WEBRTC_ROOT_DIR}' from environment variable")
else() 
  set(WEBRTC_ROOT_DIR
    ${PROJECT_SOURCE_DIR}/libs/webrtc/src
    CACHE PATH
    "WebRTC root directory."
  )
endif()

if (DEFINED ENV{WEBRTC_LIBRARY_DIR})
  set(WEBRTC_LIBRARY_DIR $ENV{WEBRTC_LIBRARY_DIR})
  message("WEBRTC_LIBRARY_DIR = '${WEBRTC_LIBRARY_DIR}' from environment variable")
else() 
  set(WEBRTC_LIBRARY_DIR
    ${PROJECT_SOURCE_DIR}/out/Release
    CACHE PATH
    "WebRTC output directory"
    )
endif()

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

# BoringSSL
find_library(_WEBRTC_BORING_SSL_LIBRARY
  NAMES boringssl
  PATHS 
    ${WEBRTC_LIBRARY_DIR}
    ${WEBRTC_LIBRARY_DIR}/obj/third_party/boringssl
  )
  
if (_WEBRTC_BORING_SSL_LIBRARY)
  list(APPEND WEBRTC_BORING_SSL_LIBRARIES ${_WEBRTC_BORING_SSL_LIBRARY})
else()
  message(FATAL_ERROR "BoringSSL library not found in WebRTC")
endif()

if (MSVC)
  find_library(_WEBRTC_BORING_SSL_ASM_LIBRARY
    NAMES boringssl_asm
    PATHS 
      ${WEBRTC_LIBRARY_DIR}
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/boringssl
    )
  
  if (_WEBRTC_BORING_SSL_ASM_LIBRARY)
    list(APPEND WEBRTC_BORING_SSL_LIBRARIES ${_WEBRTC_BORING_SSL_ASM_LIBRARY})
  else()
    message(FATAL_ERROR "BoringSSL asm library not found in WebRTC")
  endif()
endif(MSVC)

find_path(WEBRTC_BORING_SSL_INCLUDE
  NAMES
  	openssl/ssl.h
  PATHS
  	${WEBRTC_ROOT_DIR}/third_party/boringssl/src/include
  )
  

# ============================================================================
# Definitions
# ============================================================================

set(WEBRTC_DEFINES
  -DWIN32_LEAN_AND_MEAN
  -DNOMINMAX
  -DV8_DEPRECATION_WARNINGS
  -DEXPAT_RELATIVE_PATH
  -DFEATURE_ENABLE_VOICEMAIL
  -DGTEST_RELATIVE_PATH
  -DJSONCPP_RELATIVE_PATH
  -DLOGGING=1
  -DSRTP_RELATIVE_PATH
  -DFEATURE_ENABLE_SSL
  -DFEATURE_ENABLE_PSTN
  -DHAVE_SCTP
  -DHAVE_SRTP
  -DHAVE_WEBRTC_VIDEO
  -DHAVE_WEBRTC_VOICE
  -DUSE_WEBRTC_DEV_BRANCH
  -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS
  -DCHROMIUM_BUILD
  -DTOOLKIT_VIEWS=1
  -DUSE_AURA=1
  -DUSE_ASH=1
  -DUSE_DEFAULT_RENDER_THEME=1
  -DUSE_LIBJPEG_TURBO=1
  -DENABLE_ONE_CLICK_SIGNIN
  -DENABLE_PRE_SYNC_BACKUP
  -DENABLE_REMOTING=1
  -DENABLE_WEBRTC=1
  -DENABLE_PEPPER_CDMS
  -DENABLE_CONFIGURATION_POLICY
  -DENABLE_NOTIFICATIONS
  -DENABLE_HIDPI=1
  -DDONT_EMBED_BUILD_METADATA
  -DNO_TCMALLOC
  -DALLOCATOR_SHIM
  -DENABLE_TASK_MANAGER=1
  -DENABLE_EXTENSIONS=1
  -DENABLE_PLUGIN_INSTALLATION=1
  -DENABLE_PLUGINS=1
  -DENABLE_SESSION_SERVICE=1
  -DENABLE_THEMES=1
  -DENABLE_AUTOFILL_DIALOG=1
  -DENABLE_BACKGROUND=1
  -DENABLE_GOOGLE_NOW=1
  -DCLD_VERSION=2
  -DENABLE_PRINTING=1
  -DENABLE_BASIC_PRINTING=1
  -DENABLE_PRINT_PREVIEW=1
  -DENABLE_SPELLCHECK=1
  -DENABLE_CAPTIVE_PORTAL_DETECTION=1
  -DENABLE_APP_LIST=1
  -DENABLE_SETTINGS_APP=1
  -DENABLE_SUPERVISED_USERS=1
  -DENABLE_MDNS=1
  -DENABLE_SERVICE_DISCOVERY=1
  -DENABLE_WIFI_BOOTSTRAPPING=1
  -DV8_USE_EXTERNAL_STARTUP_DATA
  -DLIBPEERCONNECTION_LIB=1
  -DUSE_LIBPCI=1
  -DUSE_OPENSSL=1
  -DNVALGRIND
  -DDYNAMIC_ANNOTATIONS_ENABLED=0
  )


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
