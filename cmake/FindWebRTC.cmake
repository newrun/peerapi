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
    "webrtc-checkout/src"
    CACHE PATH
    "WebRTC root directory."
  )
endif()

if (DEFINED ENV{WEBRTC_LIBRARY_DIR})
  set(WEBRTC_LIBRARY_DIR $ENV{WEBRTC_LIBRARY_DIR})
  message("WEBRTC_LIBRARY_DIR = '${WEBRTC_LIBRARY_DIR}' from environment variable")
else()
  set(WEBRTC_LIBRARY_DIR ${WEBRTC_ROOT_DIR}/out/Release)
endif()

if (DEFINED ENV{WEBRTC_LIBRARY_DIR_DEBUG})
  set(WEBRTC_LIBRARY_DIR_DEBUG $ENV{WEBRTC_LIBRARY_DIR_DEBUG})
  message("WEBRTC_LIBRARY_DIR = '${WEBRTC_LIBRARY_DIR}' from environment variable")
else()
  set(WEBRTC_LIBRARY_DIR_DEBUG ${WEBRTC_ROOT_DIR}/out/Debug)
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
    "libjingle_peerconnection"
    "field_trial_default"
    "jsoncpp"
    "rtc_media"
    "rtc_base_approved"
    "webrtc_common"
    "video_render_module"
    "webrtc_utility"
    "audio_coding_module"
    "cng"
    "common_audio"
    "system_wrappers"
    "openmax_dl"
    "common_audio_sse2"
    "audio_encoder_interface"
    "g711"
    "pcm16b"
    "ilbc"
    "webrtc_opus"
    "opus"
    "g722"
    "isac"
    "audio_decoder_interface"
    "isac_common"
    "red"
    "rtc_event_log"
    "rtc_event_log_proto"
    "protobuf_lite"
    "neteq"
    "rent_a_codec"
    "media_file"
    "common_video"
    "libjpeg"
    "webrtc"
    "voice_engine"
    "audio_conference_mixer"
    "audio_processing"
    "audioproc_debug_proto"
    "audio_processing_sse2"
    "audio_device"
    "bitrate_controller"
    "paced_sender"
    "rtp_rtcp"
    "remote_bitrate_estimator"
    "congestion_controller"
    "video_capture_module"
    "video_processing"
    "video_processing_sse2"
    "webrtc_video_coding"
    "webrtc_h264"
    "webrtc_i420"
    "video_coding_utility"
    "webrtc_vp8"
    "libvpx"
    "libvpx_intrinsics_mmx"
    "libvpx_intrinsics_sse2"
    "libvpx_intrinsics_ssse3"
    "libvpx_intrinsics_sse4_1"
    "libvpx_intrinsics_avx"
    "libvpx_intrinsics_avx2"
    "webrtc_vp9"
    "rtc_sound"
    "rtc_base"
    "boringssl"
    "metrics_default"
    "rtc_xmllite"
    "expat"
    "rtc_xmpp"
    "rtc_p2p"
    "usrsctplib"
    "video_capture_module_internal_impl"
    "directshow_baseclasses"
    "video_render_module_internal_impl"
    "rtc_pc"
    "libsrtp"

    "libyuv yuv"
  )

if (MSVC)
  list(APPEND _WEBRTC_LIB_NAMES
    "boringssl_asm"
    )
endif(MSVC)

foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)
  find_library(_WEBRTC_LIB_PATH
    NAMES ${lib}
    PATHS 
      ${WEBRTC_LIBRARY_DIR}
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/api
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/base
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/common_audio
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/common_video
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/libjingle/xmllite
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/libjingle/xmpp
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/media
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/modules
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/modules/video_coding/utility
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/modules/video_coding/codecs/vp8
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/modules/video_coding/codecs/vp9
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/p2p
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/pc
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/sound
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/system_wrappers
      ${WEBRTC_LIBRARY_DIR}/obj/webrtc/voice_engine
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/boringssl
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/expat
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/libjpeg_turbo
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/libsrtp
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/libvpx
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/jsoncpp
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/openmax_dl/dl
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/opus
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/protobuf
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/usrsctp
      ${WEBRTC_LIBRARY_DIR}/obj/third_party/winsdk_samples
    )
  if (_WEBRTC_LIB_PATH)
    list(APPEND
      WEBRTC_LIBRARIES
      optimized ${_WEBRTC_LIB_PATH}
      )
  else(_WEBRTC_LIB_PATH)
    message("\nCurrent WEBRTC_ROOT_DIR is '${WEBRTC_ROOT_DIR}'")
    message("Current WEBRTC_LIBRARY_DIR is '${WEBRTC_LIBRARY_DIR}'\n")
    message(FATAL_ERROR "WebRTC module '${lib}' was not found. "
           "Check 'WEBRTC_ROOT_DIR' and 'WEBRTC_LIBRARY_DIR'.\n")
  endif()
endforeach()
  
# Optional DEBUG library
foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)
  find_library(_WEBRTC_LIB_PATH
    NAMES ${lib}
    PATHS 
      ${WEBRTC_LIBRARY_DIR_DEBUG}
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/api
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/base
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/common_audio
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/common_video
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/libjingle/xmllite
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/libjingle/xmpp
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/media
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/modules
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/modules/video_coding/utility
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/modules/video_coding/codecs/vp8
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/modules/video_coding/codecs/vp9
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/p2p
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/pc
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/sound
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/system_wrappers
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/webrtc/voice_engine
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/boringssl
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/expat
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/libjpeg_turbo
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/libsrtp
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/libvpx
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/jsoncpp
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/openmax_dl/dl
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/opus
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/protobuf
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/usrsctp
      ${WEBRTC_LIBRARY_DIR_DEBUG}/obj/third_party/winsdk_samples
    )
  if (_WEBRTC_LIB_PATH)
    list(APPEND
      WEBRTC_LIBRARIES
      debug ${_WEBRTC_LIB_PATH}
      )
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
    -DV8_DEPRECATION_WARNINGS -DCLD_VERSION=2
    -DCHROMIUM_BUILD
    -DUSE_LIBJPEG_TURBO=1 -DENABLE_WEBRTC=1
    -DENABLE_MEDIA_ROUTER=1 -DENABLE_PEPPER_CDMS
    -DENABLE_CONFIGURATION_POLICY -DENABLE_NOTIFICATIONS
    -DENABLE_TOPCHROME_MD=1 -DFIELDTRIAL_TESTING_ENABLED
    -DENABLE_TASK_MANAGER=1 -DENABLE_EXTENSIONS=1 -DENABLE_PDF=1
    -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_PLUGINS=1
    -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1 -DENABLE_AUTOFILL_DIALOG=1
    -DENABLE_PRINTING=1 -DENABLE_BASIC_PRINTING=1 -DENABLE_PRINT_PREVIEW=1
    -DENABLE_SPELLCHECK=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1
    -DENABLE_APP_LIST=1
    -DENABLE_SETTINGS_APP=1 -DENABLE_SUPERVISED_USERS=1 -DENABLE_MDNS=1
    -DENABLE_SERVICE_DISCOVERY=1 -DV8_USE_EXTERNAL_STARTUP_DATA
    -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD -DSAFE_BROWSING_DB_LOCAL
    -DUSE_LIBPCI=1
    -DUSE_OPENSSL=1 -DNVALGRIND -DDYNAMIC_ANNOTATIONS_ENABLED=0

    -DUSE_DEFAULT_RENDER_THEME=1
  )


if (MSVC)
  list(APPEND WEBRTC_DEFINES
    -DNOMINMAX -DPSAPI_VERSION=1 -D_CRT_RAND_S
    -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS
    -D_ATL_NO_OPENGL -D_SECURE_ATL -D_HAS_EXCEPTIONS=0
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
    -DNO_TCMALLOC -D__STD_C
    -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_WARNINGS
    -DENABLE_CAPTIVE_PORTAL_DETECTION=1
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_UNICODE -DUNICODE
    -DWEBRTC_WIN
  )
elseif(UNIX)
  list(APPEND WEBRTC_DEFINES
    -DWEBRTC_POSIX 
    -DCARBON_DEPRECATED=YES 
    -DDISABLE_DYNAMIC_CAST -D_REENTRANT
    -DUSE_BROWSER_SPELLCHECKER=1
  )

  if(APPLE)
    list(APPEND WEBRTC_DEFINES
      -DWEBRTC_MAC 
    )
  else()
    list(APPEND WEBRTC_DEFINES
      -DWEBRTC_LINUX
      -DDUI_COMPOSITOR_IMAGE_TRANSPORT
      -DUSE_PANGO=1 -DUSE_CAIRO=1
      -DUSE_X11=1 -DUSE_GLIB=1 -DUSE_NSS_CERTS=1
    )
  endif()
endif()



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
