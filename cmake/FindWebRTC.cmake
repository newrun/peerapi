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

message("Check WebRTC library")

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

list(APPEND WEBRTC_INCLUDE_DIR
  "${WEBRTC_INCLUDE_DIR}"
  "${WEBRTC_INCLUDE_DIR}/webrtc"
  "${WEBRTC_INCLUDE_DIR}/third_party"
  "${WEBRTC_INCLUDE_DIR}/third_party/webrtc"
  "${WEBRTC_INCLUDE_DIR}/third_party/wtl/include"
  "${WEBRTC_INCLUDE_DIR}/third_party/jsoncpp/overrides/include"
  "${WEBRTC_INCLUDE_DIR}/third_party/jsoncpp/source/include"
  )

# ============================================================================
# Find WebRTC libries
#   webrtc -> webrtc.lib or libwebrtc.a
#   libyuv yuv -> libyuv.lib or libyuv.a
#   ...
# ============================================================================

set (WEBRTC_LIBRARIES)

if (MSVC)
  list(APPEND _WEBRTC_LIB_NAMES
      "obj/webrtc/api/libjingle_peerconnection.lib"
      "obj/webrtc/system_wrappers/field_trial_default.lib"
      "obj/third_party/jsoncpp/jsoncpp.lib"
      "obj/webrtc/media/rtc_media.lib"
      "obj/webrtc/base/rtc_base_approved.lib"
      "obj/webrtc/webrtc_common.lib"
      "obj/webrtc/webrtc.lib"
      "obj/webrtc/system_wrappers/system_wrappers.lib"
      "obj/webrtc/voice_engine/voice_engine.lib"
      "obj/webrtc/common_audio/common_audio.lib"
      "obj/third_party/openmax_dl/dl/openmax_dl.lib"
      "obj/webrtc/common_audio/common_audio_sse2.lib"
      "obj/webrtc/modules/audio_coding_module.lib"
      "obj/webrtc/modules/cng.lib"
      "obj/webrtc/modules/audio_encoder_interface.lib"
      "obj/webrtc/modules/g711.lib"
      "obj/webrtc/modules/pcm16b.lib"
      "obj/webrtc/modules/ilbc.lib"
      "obj/webrtc/modules/webrtc_opus.lib"
      "obj/third_party/opus/opus.lib"
      "obj/webrtc/modules/g722.lib"
      "obj/webrtc/modules/isac.lib"
      "obj/webrtc/modules/audio_decoder_interface.lib"
      "obj/webrtc/modules/isac_common.lib"
      "obj/webrtc/modules/red.lib"
      "obj/webrtc/rtc_event_log.lib"
      "obj/webrtc/rtc_event_log_proto.lib"
      "obj/third_party/protobuf/protobuf_lite.lib"
      "obj/webrtc/modules/neteq.lib"
      "obj/webrtc/modules/builtin_audio_decoder_factory.lib"
      "obj/webrtc/modules/audio_decoder_factory_interface.lib"
      "obj/webrtc/modules/rent_a_codec.lib"
      "obj/webrtc/modules/audio_conference_mixer.lib"
      "obj/webrtc/modules/audio_processing.lib"
      "obj/webrtc/modules/audioproc_debug_proto.lib"
      "obj/webrtc/modules/audio_processing_sse2.lib"
      "obj/webrtc/modules/webrtc_utility.lib"
      "obj/webrtc/modules/media_file.lib"
      "obj/webrtc/modules/audio_device.lib"
      "obj/webrtc/modules/bitrate_controller.lib"
      "obj/webrtc/modules/paced_sender.lib"
      "obj/webrtc/modules/rtp_rtcp.lib"
      "obj/webrtc/modules/remote_bitrate_estimator.lib"
      "obj/webrtc/modules/congestion_controller.lib"
      "obj/webrtc/common_video/common_video.lib"
      "libyuv.lib"
      "obj/third_party/libjpeg_turbo/libjpeg.lib"
      "obj/webrtc/modules/video_capture_module.lib"
      "obj/webrtc/modules/video_processing.lib"
      "obj/webrtc/modules/video_processing_sse2.lib"
      "obj/webrtc/modules/webrtc_video_coding.lib"
      "obj/webrtc/modules/webrtc_h264.lib"
      "obj/webrtc/modules/webrtc_i420.lib"
      "obj/webrtc/modules/video_coding/utility/video_coding_utility.lib"
      "obj/webrtc/modules/video_coding/codecs/vp8/webrtc_vp8.lib"
      "obj/third_party/libvpx/libvpx.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_mmx.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_sse2.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_ssse3.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_sse4_1.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_avx.lib"
      "obj/third_party/libvpx/libvpx_intrinsics_avx2.lib"
      "obj/webrtc/modules/video_coding/codecs/vp9/webrtc_vp9.lib"
      "obj/webrtc/system_wrappers/metrics_default.lib"
      "obj/webrtc/libjingle/xmllite/rtc_xmllite.lib"
      "obj/webrtc/base/rtc_base.lib"
      "obj/third_party/boringssl/boringssl.lib"
      "obj/third_party/boringssl/boringssl_asm.lib"
      "obj/third_party/expat/expat.lib"
      "obj/webrtc/libjingle/xmpp/rtc_xmpp.lib"
      "obj/webrtc/p2p/rtc_p2p.lib"
      "obj/third_party/usrsctp/usrsctplib.lib"
      "obj/webrtc/modules/video_capture_module_internal_impl.lib"
      "obj/third_party/winsdk_samples/directshow_baseclasses.lib"
      "obj/webrtc/pc/rtc_pc.lib"
      "obj/third_party/libsrtp/libsrtp.lib"
    )
elseif (APPLE)

elseif (UNIX)

else (MSVC)
    message(FATAL_ERROR "Unknown environment. \n")
endif (MSVC)


foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)
  find_library(_WEBRTC_LIB_PATH
    NAMES ${lib}
    PATHS 
      ${WEBRTC_LIBRARY_DIR}
    )
  if (_WEBRTC_LIB_PATH)
    list(APPEND
      WEBRTC_LIBRARIES
      optimized ${_WEBRTC_LIB_PATH}
      )
  else(_WEBRTC_LIB_PATH)
    message("\nCurrent WEBRTC_ROOT_DIR is '${WEBRTC_ROOT_DIR}'")
    message("Current WEBRTC_LIBRARY_DIR is '${WEBRTC_LIBRARY_DIR}'\n")
    message(FATAL_ERROR "WebRTC module '${lib}' was not found. \n"
           "Check 'WEBRTC_ROOT_DIR'\n")
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


find_path(WEBRTC_BORING_SSL_INCLUDE
  NAMES
  	openssl/ssl.h
  PATHS
  	${WEBRTC_ROOT_DIR}/third_party/boringssl/src/include
  )
  

# ============================================================================
# Definitions
# ============================================================================



if (MSVC)
  set(WEBRTC_DEFINES
      -DV8_DEPRECATION_WARNINGS
      -DNOMINMAX -DPSAPI_VERSION=1 -D_CRT_RAND_S
      -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS -DWIN32_LEAN_AND_MEAN
      -D_ATL_NO_OPENGL -D_SECURE_ATL -D_HAS_EXCEPTIONS=0
      -D_WINSOCK_DEPRECATED_NO_WARNINGS -DCHROMIUM_BUILD
      -DUSE_AURA=1 -DUSE_DEFAULT_RENDER_THEME=1
      -DUSE_LIBJPEG_TURBO=1 -DENABLE_WEBRTC=1 -DENABLE_MEDIA_ROUTER=1
      -DENABLE_PEPPER_CDMS -DENABLE_NOTIFICATIONS -DENABLE_TOPCHROME_MD=1
      -DFIELDTRIAL_TESTING_ENABLED -DNO_TCMALLOC -D__STD_C
      -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE
      -DENABLE_TASK_MANAGER=1
      -DENABLE_EXTENSIONS=1 -DENABLE_PDF=1 -DENABLE_PLUGIN_INSTALLATION=1
      -DENABLE_PLUGINS=1 -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1
      -DENABLE_PRINTING=1 -DENABLE_BASIC_PRINTING=1 -DENABLE_PRINT_PREVIEW=1
      -DENABLE_SPELLCHECK=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1
      -DENABLE_APP_LIST=1 -DENABLE_SETTINGS_APP=1 -DENABLE_SUPERVISED_USERS=1
      -DENABLE_MDNS=1 -DENABLE_SERVICE_DISCOVERY=1
      -DV8_USE_EXTERNAL_STARTUP_DATA -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD
      -DSAFE_BROWSING_DB_LOCAL -DWEBRTC_WIN -DUSE_LIBPCI=1
      -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
      -DNVALGRIND -DDYNAMIC_ANNOTATIONS_ENABLED=0 -DALLOCATOR_SHIM -D_UNICODE
      -DUNICODE
      # Removed from origianl peerconnection_client.ninja 
      # -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00
      # -DWIN32 -D_WINDOWS 
      # -DCR_CLANG_REVISION=268813-1 
      # -DNTDDI_VERSION=0x0A000000 
      # -D_USING_V110_SDK71_ 
      # -DNDEBUG
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
  message("Found WebRTC library successfully.")
  set(WEBRTC_FOUND 1)
else()
  set(WEBRTC_FOUND 0)
  message(FATAL_ERROR "WebRTC file was not found.\n"
      "Please check 'WEBRTC_ROOT_DIR'.\n")
endif()