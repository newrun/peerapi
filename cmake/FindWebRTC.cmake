# The following variables will be defined:
#
#  WEBRTC_FOUND
#  WEBRTC_DEFINES
#  WEBRTC_INCLUDE_DIR
#  WEBRTC_LIBRARIES_INTERNAL_RELEASE
#  WEBRTC_LIBRARIES_INTERNAL_DEBUG
#  WEBRTC_LIBRARIES_INTERNAL
#  WEBRTC_LIBRARIES_EXTERNAL
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
  "${WEBRTC_INCLUDE_DIR}/third_party/boringssl/src/include"
  )

# ============================================================================
# Find WebRTC libries
#   webrtc -> webrtc.lib or libwebrtc.a
#   libyuv yuv -> libyuv.lib or libyuv.a
#   ...
# ============================================================================

set (WEBRTC_LIBRARIES_INTERNAL_RELEASE)
set (WEBRTC_LIBRARIES_INTERNAL_DEBUG)
set (WEBRTC_LIBRARIES_INTERNAL)
set (WEBRTC_LIBRARIES_EXTERNAL)

if (MSVC)
  list(APPEND _WEBRTC_LIB_NAMES
      # from peerconnection_client.ninja
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
  # from peerconnection_client.ninja
  list(APPEND _WEBRTC_LIB_NAMES
      "libjingle_peerconnection.a"
      "libfield_trial_default.a"
      "libjsoncpp.a"
      "librtc_media.a"
      "librtc_base_approved.a"
      "libwebrtc_common.a"
      "libwebrtc.a" 
      "libsystem_wrappers.a"
      "libvoice_engine.a"
      "libcommon_audio.a"
      "libopenmax_dl.a"
      "libcommon_audio_sse2.a"
      "libaudio_coding_module.a"
      "libcng.a"
      "libaudio_encoder_interface.a"
      "libg711.a"
      "libpcm16b.a"
      "libilbc.a"
      "libwebrtc_opus.a"
      "libopus.a"
      "libg722.a"
      "libisac.a"
      "libaudio_decoder_interface.a"
      "libisac_common.a"
      "libred.a"
      "librtc_event_log.a"
      "librtc_event_log_proto.a"
      "libprotobuf_lite.a"
      "libneteq.a"
      "libbuiltin_audio_decoder_factory.a"
      "libaudio_decoder_factory_interface.a"
      "librent_a_codec.a"
      "libaudio_conference_mixer.a"
      "libaudio_processing.a"
      "libaudioproc_debug_proto.a"
      "libaudio_processing_sse2.a"
      "libwebrtc_utility.a"
      "libmedia_file.a"
      "libaudio_device.a"
      "libbitrate_controller.a"
      "libpaced_sender.a"
      "librtp_rtcp.a"
      "libremote_bitrate_estimator.a"
      "libcongestion_controller.a"
      "libcommon_video.a"
      "libyuv.a"
      "libjpeg_turbo.a"
      "libvideo_capture_module.a"
      "libvideo_processing.a"
      "libvideo_processing_sse2.a"
      "libwebrtc_video_coding.a"
      "libwebrtc_h264.a"
      "libwebrtc_i420.a"
      "libvideo_coding_utility.a"
      "libwebrtc_vp8.a"
      "libvpx.a"
      "libvpx_intrinsics_mmx.a"
      "libvpx_intrinsics_sse2.a"
      "libvpx_intrinsics_ssse3.a"
      "libvpx_intrinsics_sse4_1.a"
      "libvpx_intrinsics_avx.a"
      "libvpx_intrinsics_avx2.a"
      "libwebrtc_vp9.a"
      "libmetrics_default.a"
      "librtc_xmllite.a"
      "librtc_base.a"
      "libboringssl.a"
      "libboringssl_asm.a"
      "libexpat.a"
      "librtc_xmpp.a"
      "librtc_p2p.a"
      "libusrsctplib.a"
      "libvideo_capture_module_internal_impl.a"
      "librtc_pc.a"
      "libsrtp.a"
      )


elseif (UNIX)
  list(APPEND _WEBRTC_LIB_NAMES
    # from peerconnection_client.ninja
    "obj/webrtc/api/libjingle_peerconnection.a"
    "obj/webrtc/system_wrappers/libfield_trial_default.a"
    "obj/chromium/src/third_party/jsoncpp/libjsoncpp.a"
    "obj/webrtc/media/librtc_media.a"
    "obj/webrtc/base/librtc_base_approved.a"
    "obj/chromium/src/base/third_party/libevent/libevent.a"
    "obj/webrtc/libwebrtc_common.a"
    "obj/webrtc/libwebrtc.a"
    "obj/webrtc/system_wrappers/libsystem_wrappers.a"
    "obj/webrtc/system_wrappers/libcpu_features_linux.a"
    "obj/webrtc/voice_engine/libvoice_engine.a"
    "obj/webrtc/common_audio/libcommon_audio.a"
    "obj/chromium/src/third_party/openmax_dl/dl/libopenmax_dl.a"
    "obj/webrtc/common_audio/libcommon_audio_sse2.a"
    "obj/webrtc/modules/libaudio_coding_module.a"
    "obj/webrtc/modules/libcng.a"
    "obj/webrtc/modules/libaudio_encoder_interface.a"
    "obj/webrtc/modules/libg711.a"
    "obj/webrtc/modules/libpcm16b.a"
    "obj/webrtc/modules/libilbc.a"
    "obj/webrtc/modules/libwebrtc_opus.a"
    "obj/chromium/src/third_party/opus/libopus.a"
    "obj/webrtc/modules/libg722.a"
    "obj/webrtc/modules/libisac.a"
    "obj/webrtc/modules/libaudio_decoder_interface.a"
    "obj/webrtc/modules/libisac_common.a"
    "obj/webrtc/modules/libred.a"
    "obj/webrtc/librtc_event_log.a"
    "obj/webrtc/librtc_event_log_proto.a"
    "obj/chromium/src/third_party/protobuf/libprotobuf_lite.a"
    "obj/webrtc/modules/libneteq.a"
    "obj/webrtc/modules/libbuiltin_audio_decoder_factory.a"
    "obj/webrtc/modules/libaudio_decoder_factory_interface.a"
    "obj/webrtc/modules/librent_a_codec.a"
    "obj/webrtc/modules/libaudio_conference_mixer.a"
    "obj/webrtc/modules/libaudio_processing.a"
    "obj/webrtc/modules/libaudioproc_debug_proto.a"
    "obj/webrtc/modules/libaudio_processing_sse2.a"
    "obj/webrtc/modules/libwebrtc_utility.a"
    "obj/webrtc/modules/libmedia_file.a"
    "obj/webrtc/modules/libaudio_device.a"
    "obj/webrtc/modules/libbitrate_controller.a"
    "obj/webrtc/modules/libpaced_sender.a"
    "obj/webrtc/modules/librtp_rtcp.a"
    "obj/webrtc/modules/libremote_bitrate_estimator.a"
    "obj/webrtc/modules/libcongestion_controller.a"
    "obj/webrtc/common_video/libcommon_video.a"
    "libyuv.a"
    "obj/chromium/src/third_party/libjpeg_turbo/libjpeg_turbo.a"
    "obj/webrtc/modules/libvideo_capture_module.a"
    "obj/webrtc/modules/libvideo_processing.a"
    "obj/webrtc/modules/libvideo_processing_sse2.a"
    "obj/webrtc/modules/libwebrtc_video_coding.a"
    "obj/webrtc/modules/libwebrtc_h264.a"
    "obj/webrtc/modules/libwebrtc_i420.a"
    "obj/webrtc/modules/video_coding/utility/libvideo_coding_utility.a"
    "obj/webrtc/modules/video_coding/codecs/vp8/libwebrtc_vp8.a"
    "obj/chromium/src/third_party/libvpx/libvpx.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_mmx.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_sse2.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_ssse3.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_sse4_1.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_avx.a"
    "obj/chromium/src/third_party/libvpx/libvpx_intrinsics_avx2.a"
    "obj/webrtc/modules/video_coding/codecs/vp9/libwebrtc_vp9.a"
    "obj/webrtc/system_wrappers/libmetrics_default.a"
    "obj/webrtc/libjingle/xmllite/librtc_xmllite.a"
    "obj/webrtc/base/librtc_base.a"
    "obj/chromium/src/third_party/boringssl/libboringssl.a"
    "obj/chromium/src/third_party/boringssl/libboringssl_asm.a"
    "obj/webrtc/libjingle/xmpp/librtc_xmpp.a"
    "obj/webrtc/p2p/librtc_p2p.a"
    "obj/chromium/src/third_party/usrsctp/libusrsctplib.a"
    "obj/webrtc/modules/libvideo_capture_module_internal_impl.a"
    "obj/webrtc/pc/librtc_pc.a"
    "obj/chromium/src/third_party/libsrtp/libsrtp.a"
    )
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
    list(APPEND WEBRTC_LIBRARIES_INTERNAL_RELEASE ${_WEBRTC_LIB_PATH})
    list(APPEND WEBRTC_LIBRARIES_INTERNAL optimized ${_WEBRTC_LIB_PATH})
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
    list(APPEND WEBRTC_LIBRARIES_INTERNAL_DEBUG ${_WEBRTC_LIB_PATH})
    list(APPEND WEBRTC_LIBRARIES_INTERNAL debug ${_WEBRTC_LIB_PATH})
  endif()
endforeach()

if(WIN32 AND MSVC)
  set(_WEBRTC_EXTERNAL_LIBRARIES
        Secur32.lib Winmm.lib msdmo.lib dmoguids.lib wmcodecdspuuid.lib
        wininet.lib dnsapi.lib version.lib ws2_32.lib Strmiids.lib)
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${_WEBRTC_EXTERNAL_LIBRARIES}) 
elseif (APPLE)
  find_library(FOUNDATION_LIBRARY Foundation)
  find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
  find_library(CORE_SERVICES_LIBRARY CoreServices)
  find_library(CORE_AUDIO_LIBRARY CoreAudio)
  find_library(CORE_AUDIO_TOOLBOX_LIBRARY AudioToolBox)
  find_library(CORE_GRAPHICSLIBRARY CoreGraphics)
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${FOUNDATION_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_FOUNDATION_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_SERVICES_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_AUDIO_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_AUDIO_TOOLBOX_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_GRAPHICSLIBRARY})    
elseif (UNIX)
  find_package(Threads REQUIRED)
  find_library(X11_LIBRARY NAMES X11)

  list(APPEND WEBRTC_LIBRARIES_EXTERNAL
              ${CMAKE_THREAD_LIBS_INIT}
              ${X11_LIBRARY}
              ${CMAKE_DL_LIBS})
endif()
  

# ============================================================================
# Definitions
# ============================================================================

if (MSVC)
  set(WEBRTC_DEFINES
      # from peerconnection_client.ninja
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
      # Skipped from origianl peerconnection_client.ninja 
      # -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00
      # -DWIN32 -D_WINDOWS 
      # -DCR_CLANG_REVISION=268813-1 
      # -DNTDDI_VERSION=0x0A000000 
      # -D_USING_V110_SDK71_ 
      # -DNDEBUG
    )
elseif (APPLE)
  # from peerconnection_client.ninja
  set(WEBRTC_DEFINES
      -DV8_DEPRECATION_WARNINGS
      -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORE=0 -DCHROMIUM_BUILD
      -DUSE_LIBJPEG_TURBO=1 -DENABLE_WEBRTC=1
      -DENABLE_MEDIA_ROUTER=1 -DENABLE_PEPPER_CDMS -DENABLE_NOTIFICATIONS
      -DENABLE_TOPCHROME_MD=1 -DFIELDTRIAL_TESTING_ENABLED
      -DENABLE_TASK_MANAGER=1 -DENABLE_EXTENSIONS=1 -DENABLE_PDF=1
      -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_PLUGINS=1
      -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1 -DENABLE_PRINTING=1
      -DENABLE_BASIC_PRINTING=1 -DENABLE_PRINT_PREVIEW=1
      -DENABLE_SPELLCHECK=1 -DUSE_BROWSER_SPELLCHECKER=1
      -DENABLE_CAPTIVE_PORTAL_DETECTION=1 -DENABLE_APP_LIST=1
      -DENABLE_SETTINGS_APP=1 -DENABLE_SUPERVISED_USERS=1
      -DENABLE_SERVICE_DISCOVERY=1 -DV8_USE_EXTERNAL_STARTUP_DATA
      -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD -DSAFE_BROWSING_DB_LOCAL
      -DWEBRTC_MAC -DCARBON_DEPRECATED=YES -DHASH_NAMESPACE=__gnu_cxx
      -DWEBRTC_POSIX -DDISABLE_DYNAMIC_CAST -D_REENTRANT -DUSE_LIBPCI=1
      -DNVALGRIND -DDYNAMIC_ANNOTATIONS_ENABLED=0

#      -DCR_CLANG_REVISION=268813-1 
#      -DNDEBUG 
    )

elseif (UNIX)
  set(WEBRTC_DEFINES
    # from peerconnection_client.ninja
    -DV8_DEPRECATION_WARNINGS -D_FILE_OFFSET_BITS=64 -DCHROMIUM_BUILD
    -DUI_COMPOSITOR_IMAGE_TRANSPORT
    -DUSE_AURA=1 -DUSE_PANGO=1 -DUSE_CAIRO=1 -DUSE_DEFAULT_RENDER_THEME=1
    -DUSE_LIBJPEG_TURBO=1 -DUSE_X11=1 -DUSE_CLIPBOARD_AURAX11=1
    -DENABLE_WEBRTC=1 -DENABLE_MEDIA_ROUTER=1 -DENABLE_PEPPER_CDMS
    -DENABLE_NOTIFICATIONS -DENABLE_TOPCHROME_MD=1 -DUSE_UDEV
    -DFIELDTRIAL_TESTING_ENABLED -DENABLE_TASK_MANAGER=1
    -DENABLE_EXTENSIONS=1 -DENABLE_PDF=1 -DENABLE_PLUGINS=1
    -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1 -DENABLE_PRINTING=1
    -DENABLE_BASIC_PRINTING=1 -DENABLE_PRINT_PREVIEW=1
    -DENABLE_SPELLCHECK=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1
    -DENABLE_APP_LIST=1 -DENABLE_SETTINGS_APP=1 -DENABLE_SUPERVISED_USERS=1
    -DENABLE_MDNS=1 -DENABLE_SERVICE_DISCOVERY=1
    -DV8_USE_EXTERNAL_STARTUP_DATA -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD
    -DSAFE_BROWSING_DB_LOCAL -DWEBRTC_LINUX -DHASH_NAMESPACE=__gnu_cxx
    -DWEBRTC_POSIX -DDISABLE_DYNAMIC_CAST -D_REENTRANT -DUSE_LIBPCI=1
    -DUSE_GLIB=1 -DUSE_NSS_CERTS=1 -DNVALGRIND
    -DDYNAMIC_ANNOTATIONS_ENABLED=0

    # Skipped from origianl peerconnection_client.ninja 
#    -DCR_CLANG_REVISION=268813-1
#    -DNDEBUG 
    )

endif (MSVC)


# ============================================================================
# Validation
# ============================================================================

if (WEBRTC_INCLUDE_DIR)
  message("Found WebRTC library successfully.")
  set(WEBRTC_FOUND 1)
else()
  set(WEBRTC_FOUND 0)
  message(FATAL_ERROR "WebRTC file was not found.\n"
      "Please check 'WEBRTC_ROOT_DIR'.\n")
endif()