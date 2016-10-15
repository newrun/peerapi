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
    ""
    CACHE PATH
    "WebRTC root directory."
    )
endif()

if (DEFINED ENV{DEPOT_TOOLS_DIR})
  set(DEPOT_TOOLS_DIR $ENV{DEPOT_TOOLS_DIR})
  message("DEPOT_TOOLS_DIR = '${DEPOT_TOOLS_DIR}' from environment variable")
else() 
  set(DEPOT_TOOLS_DIR
    ""
    CACHE PATH
    "WebRTC depot_tools directory."
    )
endif()

if ( "${WEBRTC_ROOT_DIR}" STREQUAL "")
    message(FATAL_ERROR "A WEBRTC_ROOT_DIR is requred. \n"
                        " ex) cmake .. -DWEBRTC_ROOT_DIR=/dir/webrtc-checkout/src -DDEPOT_TOOLS_DIR=/dir/depot_tools\n")
endif()

if ( "${DEPOT_TOOLS_DIR}" STREQUAL "")
    message(FATAL_ERROR "A DEPOT_TOOLS_DIR is requred.\n"
                        " ex) cmake .. -DWEBRTC_ROOT_DIR=/dir/webrtc-checkout/src -DDEPOT_TOOLS_DIR=/dir/depot_tools\n")
endif()

file(TO_CMAKE_PATH ${WEBRTC_ROOT_DIR} WEBRTC_ROOT_DIR)
file(TO_CMAKE_PATH ${DEPOT_TOOLS_DIR} DEPOT_TOOLS_DIR)

set (WEBRTC_ROOT_DIR "${WEBRTC_ROOT_DIR}/")
set (DEPOT_TOOLS_DIR "${DEPOT_TOOLS_DIR}/")

set( WEBRTC_OUT_DIR "out/Peers")
set( WEBRTC_OUT_DIR_DEBUG "out/PeersDebug")
set( WEBRTC_LIBRARY_DIR ${WEBRTC_ROOT_DIR}/${WEBRTC_OUT_DIR})
set( WEBRTC_LIBRARY_DIR_DEBUG ${WEBRTC_ROOT_DIR}${WEBRTC_OUT_DIR_DEBUG})


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
  # from peerconnection_client.ninja, WebRTC branch-heads/55

  list(APPEND _WEBRTC_LIB_NAMES
    obj/third_party/libyuv/libyuv.lib obj/webrtc/api/libjingle_peerconnection.lib obj/webrtc/system_wrappers/field_trial_default.lib obj/webrtc/system_wrappers/metrics_default.lib obj/third_party/libjpeg_turbo/libjpeg.lib obj/third_party/libjpeg_turbo/simd.lib obj/third_party/libjpeg_turbo/simd_asm.lib obj/webrtc/webrtc_common.lib obj/webrtc/base/rtc_base_approved.lib obj/webrtc/modules/audio_coding/audio_encoder_interface.lib obj/webrtc/call/call.lib obj/webrtc/audio/audio.lib obj/webrtc/system_wrappers/system_wrappers.lib obj/webrtc/voice_engine/voice_engine.lib obj/webrtc/modules/audio_coding/audio_coding.lib obj/webrtc/modules/audio_coding/webrtc_opus.lib obj/third_party/opus/opus.lib obj/webrtc/modules/audio_coding/audio_decoder_interface.lib obj/webrtc/modules/audio_coding/cng.lib obj/webrtc/common_audio/common_audio.lib obj/third_party/openmax_dl/dl/dl.lib obj/webrtc/common_audio/common_audio_sse2.lib obj/webrtc/modules/audio_coding/g711.lib obj/webrtc/modules/audio_coding/pcm16b.lib obj/webrtc/modules/audio_coding/ilbc.lib obj/webrtc/modules/audio_coding/isac.lib obj/webrtc/modules/audio_coding/isac_common.lib obj/webrtc/modules/audio_coding/g722.lib obj/webrtc/modules/audio_coding/red.lib obj/webrtc/modules/audio_coding/ana_config_proto.lib obj/third_party/protobuf/protobuf_lite.lib obj/webrtc/modules/audio_coding/ana_debug_dump_proto.lib obj/webrtc/modules/audio_coding/neteq.lib obj/webrtc/modules/audio_coding/builtin_audio_decoder_factory.lib obj/webrtc/modules/audio_coding/audio_decoder_factory_interface.lib obj/webrtc/modules/audio_coding/rent_a_codec.lib obj/webrtc/voice_engine/level_indicator.lib obj/webrtc/modules/audio_conference_mixer/audio_conference_mixer.lib obj/webrtc/modules/audio_processing/audio_processing.lib obj/webrtc/modules/audio_processing/audioproc_debug_proto.lib obj/webrtc/modules/audio_processing/audio_processing_sse2.lib obj/webrtc/modules/utility/utility.lib obj/webrtc/base/rtc_task_queue.lib obj/webrtc/modules/media_file/media_file.lib obj/webrtc/modules/audio_device/audio_device.lib obj/webrtc/modules/bitrate_controller/bitrate_controller.lib obj/webrtc/modules/pacing/pacing.lib obj/webrtc/modules/rtp_rtcp/rtp_rtcp.lib obj/webrtc/common_video/common_video.lib obj/webrtc/modules/remote_bitrate_estimator/remote_bitrate_estimator.lib obj/webrtc/logging/rtc_event_log_impl.lib obj/webrtc/logging/rtc_event_log_proto.lib obj/webrtc/modules/congestion_controller/congestion_controller.lib obj/webrtc/video/video.lib obj/webrtc/modules/video_capture/video_capture_module.lib obj/webrtc/modules/video_coding/video_coding.lib obj/webrtc/modules/video_coding/video_coding_utility.lib obj/webrtc/modules/video_coding/webrtc_h264.lib obj/webrtc/modules/video_coding/webrtc_i420.lib obj/webrtc/modules/video_coding/webrtc_vp8.lib obj/third_party/libvpx/libvpx.lib obj/third_party/libvpx/libvpx_yasm.lib obj/webrtc/modules/video_coding/webrtc_vp9.lib obj/webrtc/modules/video_processing/video_processing.lib obj/webrtc/modules/video_processing/video_processing_sse2.lib obj/webrtc/media/rtc_media.lib obj/third_party/usrsctp/usrsctp.lib obj/third_party/boringssl/boringssl.lib obj/third_party/boringssl/boringssl_asm.lib obj/webrtc/modules/video_capture/video_capture_internal_impl.lib obj/third_party/winsdk_samples/winsdk_samples.lib obj/webrtc/libjingle/xmllite/rtc_xmllite.lib obj/third_party/expat/expat.lib obj/webrtc/base/rtc_base.lib obj/webrtc/libjingle/xmpp/rtc_xmpp.lib obj/webrtc/p2p/rtc_p2p.lib obj/webrtc/pc/rtc_pc.lib obj/third_party/libsrtp/libsrtp.lib obj/webrtc/stats/rtc_stats.lib
  )

  list(APPEND _WEBRTC_OBJ_NAMES
    obj/third_party/jsoncpp/jsoncpp/json_reader.obj obj/third_party/jsoncpp/jsoncpp/json_value.obj obj/third_party/jsoncpp/jsoncpp/json_writer.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor_impl.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/bitrate_controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/channel_controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/controller_manager.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/debug_dump_writer.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/dtx_controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/fec_controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/frame_length_controller.obj obj/webrtc/modules/audio_coding/audio_network_adaptor/smoothing_filter.obj obj/third_party/libvpx/libvpx_intrinsics_avx/vp9_diamond_search_sad_avx.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/vp9_error_intrin_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/fwd_txfm_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/loopfilter_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/sad4d_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/sad_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_impl_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_avx2/vpx_subpixel_8t_intrin_avx2.obj obj/third_party/libvpx/libvpx_intrinsics_mmx/idct_blk_mmx.obj obj/third_party/libvpx/libvpx_intrinsics_mmx/vp8_enc_stubs_mmx.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/idct_blk_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/denoising_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_enc_stubs_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_quantize_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_idct_intrin_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_dct_intrin_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_denoiser_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_highbd_block_error_intrin_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_quantize_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/avg_intrin_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/fwd_txfm_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_loopfilter_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_quantize_intrin_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_variance_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/inv_txfm_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/loopfilter_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/quantize_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/sum_squares_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse2/variance_sse2.obj obj/third_party/libvpx/libvpx_intrinsics_sse4_1/quantize_sse4.obj obj/third_party/libvpx/libvpx_intrinsics_ssse3/quantize_ssse3.obj obj/third_party/libvpx/libvpx_intrinsics_ssse3/vp9_dct_ssse3.obj obj/third_party/libvpx/libvpx_intrinsics_ssse3/vpx_subpixel_8t_intrin_ssse3.obj  
  )

elseif (APPLE)
  # from api/peerconnection_unittests.ninja, WebRTC branch-heads/55
  # TODO: Remove unnecessary objects refer to peerconnection_client.ninja

  list(APPEND _WEBRTC_LIB_NAMES
    obj/webrtc/api/libjingle_peerconnection.a obj/webrtc/libwebrtc_common.a obj/webrtc/pc/librtc_pc.a obj/webrtc/system_wrappers/libmetrics_default.a obj/testing/gmock/libgmock.a obj/webrtc/base/librtc_base_approved.a obj/webrtc/modules/audio_coding/libaudio_encoder_interface.a obj/webrtc/call/libcall.a obj/webrtc/audio/libaudio.a obj/webrtc/system_wrappers/libsystem_wrappers.a obj/webrtc/voice_engine/libvoice_engine.a obj/webrtc/modules/audio_coding/libaudio_coding.a obj/webrtc/modules/audio_coding/libwebrtc_opus.a obj/third_party/opus/libopus.a obj/webrtc/modules/audio_coding/libaudio_decoder_interface.a obj/webrtc/modules/audio_coding/libcng.a obj/webrtc/common_audio/libcommon_audio.a obj/third_party/openmax_dl/dl/libdl.a obj/webrtc/common_audio/libcommon_audio_sse2.a obj/webrtc/modules/audio_coding/libg711.a obj/webrtc/modules/audio_coding/libpcm16b.a obj/webrtc/modules/audio_coding/libilbc.a obj/webrtc/modules/audio_coding/libisac.a obj/webrtc/modules/audio_coding/libisac_common.a obj/webrtc/modules/audio_coding/libg722.a obj/webrtc/modules/audio_coding/libred.a obj/webrtc/modules/audio_coding/libana_config_proto.a obj/third_party/protobuf/libprotobuf_lite.a obj/webrtc/modules/audio_coding/libana_debug_dump_proto.a obj/webrtc/modules/audio_coding/libneteq.a obj/webrtc/modules/audio_coding/libbuiltin_audio_decoder_factory.a obj/webrtc/modules/audio_coding/libaudio_decoder_factory_interface.a obj/webrtc/modules/audio_coding/librent_a_codec.a obj/webrtc/voice_engine/liblevel_indicator.a obj/webrtc/modules/audio_conference_mixer/libaudio_conference_mixer.a obj/webrtc/modules/audio_processing/libaudio_processing.a obj/webrtc/modules/audio_processing/libaudioproc_debug_proto.a obj/webrtc/modules/audio_processing/libaudio_processing_sse2.a obj/webrtc/modules/utility/libutility.a obj/webrtc/base/librtc_task_queue.a obj/webrtc/modules/media_file/libmedia_file.a obj/webrtc/modules/audio_device/libaudio_device.a obj/webrtc/modules/bitrate_controller/libbitrate_controller.a obj/webrtc/modules/pacing/libpacing.a obj/webrtc/modules/rtp_rtcp/librtp_rtcp.a obj/webrtc/common_video/libcommon_video.a obj/third_party/libyuv/libyuv.a obj/third_party/libjpeg_turbo/libjpeg.a obj/third_party/libjpeg_turbo/libsimd.a obj/third_party/libjpeg_turbo/libsimd_asm.a obj/webrtc/modules/remote_bitrate_estimator/libremote_bitrate_estimator.a obj/webrtc/logging/librtc_event_log_impl.a obj/webrtc/logging/librtc_event_log_proto.a obj/webrtc/modules/congestion_controller/libcongestion_controller.a obj/webrtc/video/libvideo.a obj/webrtc/modules/video_capture/libvideo_capture_module.a obj/webrtc/modules/video_coding/libvideo_coding.a obj/webrtc/modules/video_coding/libvideo_coding_utility.a obj/webrtc/modules/video_coding/libwebrtc_h264.a obj/webrtc/modules/video_coding/libwebrtc_i420.a obj/webrtc/modules/video_coding/libwebrtc_vp8.a obj/third_party/libvpx/libvpx.a obj/third_party/libvpx/libvpx_yasm.a obj/webrtc/modules/video_coding/libwebrtc_vp9.a obj/webrtc/modules/video_processing/libvideo_processing.a obj/webrtc/modules/video_processing/libvideo_processing_sse2.a obj/webrtc/media/librtc_media.a obj/third_party/usrsctp/libusrsctp.a obj/third_party/boringssl/libboringssl.a obj/webrtc/modules/video_capture/libvideo_capture_internal_impl.a obj/webrtc/libjingle/xmllite/librtc_xmllite.a obj/third_party/expat/libexpat.a obj/webrtc/base/librtc_base.a obj/webrtc/libjingle/xmpp/librtc_xmpp.a obj/webrtc/p2p/librtc_p2p.a obj/third_party/libsrtp/libsrtp.a obj/webrtc/stats/librtc_stats.a obj/testing/gtest/libgtest.a obj/webrtc/system_wrappers/libfield_trial_default.a
  )

  list(APPEND _WEBRTC_OBJ_NAMES
    obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor.o obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor_impl.o obj/webrtc/modules/audio_coding/audio_network_adaptor/bitrate_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/channel_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/controller_manager.o obj/webrtc/modules/audio_coding/audio_network_adaptor/debug_dump_writer.o obj/webrtc/modules/audio_coding/audio_network_adaptor/dtx_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/fec_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/frame_length_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/smoothing_filter.o obj/third_party/libvpx/libvpx_intrinsics_avx/vp9_diamond_search_sad_avx.o obj/third_party/libvpx/libvpx_intrinsics_avx2/vp9_error_intrin_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/fwd_txfm_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/loopfilter_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/sad4d_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/sad_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_impl_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/vpx_subpixel_8t_intrin_avx2.o obj/third_party/libvpx/libvpx_intrinsics_mmx/idct_blk_mmx.o obj/third_party/libvpx/libvpx_intrinsics_mmx/vp8_enc_stubs_mmx.o obj/third_party/libvpx/libvpx_intrinsics_sse2/idct_blk_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/denoising_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_enc_stubs_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_idct_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_dct_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_denoiser_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_highbd_block_error_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/avg_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/fwd_txfm_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_loopfilter_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_quantize_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_variance_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/inv_txfm_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/loopfilter_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/sum_squares_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/variance_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse4_1/quantize_sse4.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/quantize_ssse3.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/vp9_dct_ssse3.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/vpx_subpixel_8t_intrin_ssse3.o obj/third_party/boringssl/boringssl_asm/aes-x86_64.o obj/third_party/boringssl/boringssl_asm/aesni-x86_64.o obj/third_party/boringssl/boringssl_asm/bsaes-x86_64.o obj/third_party/boringssl/boringssl_asm/vpaes-x86_64.o obj/third_party/boringssl/boringssl_asm/rsaz-avx2.o obj/third_party/boringssl/boringssl_asm/rsaz-x86_64.o obj/third_party/boringssl/boringssl_asm/x86_64-mont.o obj/third_party/boringssl/boringssl_asm/x86_64-mont5.o obj/third_party/boringssl/boringssl_asm/chacha-x86_64.o obj/third_party/boringssl/boringssl_asm/p256-x86_64-asm.o obj/third_party/boringssl/boringssl_asm/md5-x86_64.o obj/third_party/boringssl/boringssl_asm/aesni-gcm-x86_64.o obj/third_party/boringssl/boringssl_asm/ghash-x86_64.o obj/third_party/boringssl/boringssl_asm/rdrand-x86_64.o obj/third_party/boringssl/boringssl_asm/sha1-x86_64.o obj/third_party/boringssl/boringssl_asm/sha256-x86_64.o obj/third_party/boringssl/boringssl_asm/sha512-x86_64.o obj/third_party/boringssl/boringssl_asm/x25519-asm-x86_64.o obj/third_party/jsoncpp/jsoncpp/json_reader.o obj/third_party/jsoncpp/jsoncpp/json_value.o obj/third_party/jsoncpp/jsoncpp/json_writer.o
    )    
elseif (UNIX)
  # from peerconnection_client.ninja, WebRTC branch-heads/55
  list(APPEND _WEBRTC_LIB_NAMES
    obj/third_party/libyuv/libyuv.a obj/webrtc/api/libjingle_peerconnection.a obj/webrtc/system_wrappers/libfield_trial_default.a obj/webrtc/system_wrappers/libmetrics_default.a obj/third_party/libjpeg_turbo/libjpeg.a obj/third_party/libjpeg_turbo/libsimd.a obj/third_party/libjpeg_turbo/libsimd_asm.a obj/webrtc/libwebrtc_common.a obj/webrtc/base/librtc_base_approved.a obj/webrtc/modules/audio_coding/libaudio_encoder_interface.a obj/webrtc/call/libcall.a obj/webrtc/audio/libaudio.a obj/webrtc/system_wrappers/libsystem_wrappers.a obj/webrtc/system_wrappers/libcpu_features_linux.a obj/webrtc/voice_engine/libvoice_engine.a obj/webrtc/modules/audio_coding/libaudio_coding.a obj/webrtc/modules/audio_coding/libwebrtc_opus.a obj/third_party/opus/libopus.a obj/webrtc/modules/audio_coding/libaudio_decoder_interface.a obj/webrtc/modules/audio_coding/libcng.a obj/webrtc/common_audio/libcommon_audio.a obj/third_party/openmax_dl/dl/libdl.a obj/webrtc/common_audio/libcommon_audio_sse2.a obj/webrtc/modules/audio_coding/libg711.a obj/webrtc/modules/audio_coding/libpcm16b.a obj/webrtc/modules/audio_coding/libilbc.a obj/webrtc/modules/audio_coding/libisac.a obj/webrtc/modules/audio_coding/libisac_common.a obj/webrtc/modules/audio_coding/libg722.a obj/webrtc/modules/audio_coding/libred.a obj/webrtc/modules/audio_coding/libana_config_proto.a obj/third_party/protobuf/libprotobuf_lite.a obj/webrtc/modules/audio_coding/libana_debug_dump_proto.a obj/webrtc/modules/audio_coding/libneteq.a obj/webrtc/modules/audio_coding/libbuiltin_audio_decoder_factory.a obj/webrtc/modules/audio_coding/libaudio_decoder_factory_interface.a obj/webrtc/modules/audio_coding/librent_a_codec.a obj/webrtc/voice_engine/liblevel_indicator.a obj/webrtc/modules/audio_conference_mixer/libaudio_conference_mixer.a obj/webrtc/modules/audio_processing/libaudio_processing.a obj/webrtc/modules/audio_processing/libaudioproc_debug_proto.a obj/webrtc/modules/audio_processing/libaudio_processing_sse2.a obj/webrtc/modules/utility/libutility.a obj/webrtc/base/librtc_task_queue.a obj/base/third_party/libevent/libevent.a obj/webrtc/modules/media_file/libmedia_file.a obj/webrtc/modules/audio_device/libaudio_device.a obj/webrtc/modules/bitrate_controller/libbitrate_controller.a obj/webrtc/modules/pacing/libpacing.a obj/webrtc/modules/rtp_rtcp/librtp_rtcp.a obj/webrtc/common_video/libcommon_video.a obj/webrtc/modules/remote_bitrate_estimator/libremote_bitrate_estimator.a obj/webrtc/logging/librtc_event_log_impl.a obj/webrtc/logging/librtc_event_log_proto.a obj/webrtc/modules/congestion_controller/libcongestion_controller.a obj/webrtc/video/libvideo.a obj/webrtc/modules/video_capture/libvideo_capture_module.a obj/webrtc/modules/video_coding/libvideo_coding.a obj/webrtc/modules/video_coding/libvideo_coding_utility.a obj/webrtc/modules/video_coding/libwebrtc_h264.a obj/webrtc/modules/video_coding/libwebrtc_i420.a obj/third_party/libvpx/libvpx_yasm.a obj/webrtc/modules/video_coding/libwebrtc_vp9.a obj/webrtc/modules/video_processing/libvideo_processing.a obj/webrtc/modules/video_processing/libvideo_processing_sse2.a obj/webrtc/media/librtc_media.a obj/third_party/usrsctp/libusrsctp.a obj/third_party/boringssl/libboringssl.a obj/webrtc/modules/video_capture/libvideo_capture_internal_impl.a obj/webrtc/libjingle/xmllite/librtc_xmllite.a obj/webrtc/base/librtc_base.a obj/webrtc/libjingle/xmpp/librtc_xmpp.a obj/webrtc/p2p/librtc_p2p.a obj/webrtc/pc/librtc_pc.a obj/third_party/libsrtp/libsrtp.a obj/webrtc/stats/librtc_stats.a
     #obj/webrtc/modules/video_coding/libwebrtc_vp8.a obj/third_party/libvpx/libvpx.a
    )

  list(APPEND _WEBRTC_OBJ_NAMES
    obj/third_party/jsoncpp/jsoncpp/json_reader.o obj/third_party/jsoncpp/jsoncpp/json_value.o obj/third_party/jsoncpp/jsoncpp/json_writer.o obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor.o obj/webrtc/modules/audio_coding/audio_network_adaptor/audio_network_adaptor_impl.o obj/webrtc/modules/audio_coding/audio_network_adaptor/bitrate_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/channel_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/controller_manager.o obj/webrtc/modules/audio_coding/audio_network_adaptor/debug_dump_writer.o obj/webrtc/modules/audio_coding/audio_network_adaptor/dtx_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/fec_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/frame_length_controller.o obj/webrtc/modules/audio_coding/audio_network_adaptor/smoothing_filter.o obj/third_party/libvpx/libvpx_intrinsics_avx/vp9_diamond_search_sad_avx.o obj/third_party/libvpx/libvpx_intrinsics_avx2/vp9_error_intrin_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/fwd_txfm_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/loopfilter_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/sad4d_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/sad_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/variance_impl_avx2.o obj/third_party/libvpx/libvpx_intrinsics_avx2/vpx_subpixel_8t_intrin_avx2.o obj/third_party/libvpx/libvpx_intrinsics_mmx/idct_blk_mmx.o obj/third_party/libvpx/libvpx_intrinsics_mmx/vp8_enc_stubs_mmx.o obj/third_party/libvpx/libvpx_intrinsics_sse2/idct_blk_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/denoising_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_enc_stubs_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp8_quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_idct_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_dct_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_denoiser_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_highbd_block_error_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/vp9_quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/avg_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/fwd_txfm_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_loopfilter_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_quantize_intrin_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/highbd_variance_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/inv_txfm_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/loopfilter_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/quantize_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/sum_squares_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse2/variance_sse2.o obj/third_party/libvpx/libvpx_intrinsics_sse4_1/quantize_sse4.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/quantize_ssse3.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/vp9_dct_ssse3.o obj/third_party/libvpx/libvpx_intrinsics_ssse3/vpx_subpixel_8t_intrin_ssse3.o obj/third_party/boringssl/boringssl_asm/aes-x86_64.o obj/third_party/boringssl/boringssl_asm/aesni-x86_64.o obj/third_party/boringssl/boringssl_asm/bsaes-x86_64.o obj/third_party/boringssl/boringssl_asm/vpaes-x86_64.o obj/third_party/boringssl/boringssl_asm/rsaz-avx2.o obj/third_party/boringssl/boringssl_asm/rsaz-x86_64.o obj/third_party/boringssl/boringssl_asm/x86_64-mont.o obj/third_party/boringssl/boringssl_asm/x86_64-mont5.o obj/third_party/boringssl/boringssl_asm/chacha-x86_64.o obj/third_party/boringssl/boringssl_asm/p256-x86_64-asm.o obj/third_party/boringssl/boringssl_asm/md5-x86_64.o obj/third_party/boringssl/boringssl_asm/aesni-gcm-x86_64.o obj/third_party/boringssl/boringssl_asm/ghash-x86_64.o obj/third_party/boringssl/boringssl_asm/rdrand-x86_64.o obj/third_party/boringssl/boringssl_asm/sha1-x86_64.o obj/third_party/boringssl/boringssl_asm/sha256-x86_64.o obj/third_party/boringssl/boringssl_asm/sha512-x86_64.o obj/third_party/boringssl/boringssl_asm/x25519-asm-x86_64.o
    )
else (MSVC)
    message(FATAL_ERROR "Unknown environment. \n")
endif (MSVC)

foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)

  set (_WEBRTC_LIB_PATH "${WEBRTC_LIBRARY_DIR}/${lib}")

  list(APPEND WEBRTC_LIBRARIES_INTERNAL_RELEASE ${_WEBRTC_LIB_PATH})
  list(APPEND WEBRTC_LIBRARIES_INTERNAL optimized ${_WEBRTC_LIB_PATH})
endforeach()

foreach (obj ${_WEBRTC_OBJ_NAMES})
  unset(_WEBRTC_OBJ_PATH CACHE)
  separate_arguments(obj)

  set (_WEBRTC_OBJ_PATH "${WEBRTC_LIBRARY_DIR}/${obj}")

  set_source_files_properties(
    ${_WEBRTC_OBJ_PATH} PROPERTIES
    EXTERNAL_OBJECT TRUE  # Identifies this as an object file
    GENERATED TRUE  # Avoids need for file to exist at configure-time
)

  list(APPEND WEBRTC_OBJECTS_INTERNAL_RELEASE ${_WEBRTC_OBJ_PATH})
  list(APPEND WEBRTC_OBJECTS_INTERNAL optimized ${_WEBRTC_OBJ_PATH})
endforeach()

  
# Optional DEBUG library
foreach (lib ${_WEBRTC_LIB_NAMES})
  unset(_WEBRTC_LIB_PATH CACHE)
  separate_arguments(lib)

  set (_WEBRTC_LIB_PATH "${WEBRTC_LIBRARY_DIR_DEBUG}/${lib}")

  list(APPEND WEBRTC_LIBRARIES_INTERNAL_DEBUG ${_WEBRTC_LIB_PATH})
  list(APPEND WEBRTC_LIBRARIES_INTERNAL debug ${_WEBRTC_LIB_PATH})
endforeach()

foreach (obj ${_WEBRTC_OBJ_NAMES})
  unset(_WEBRTC_OBJ_PATH CACHE)
  separate_arguments(obj)

  set (_WEBRTC_OBJ_PATH "${WEBRTC_LIBRARY_DIR_DEBUG}/${obj}")

  set_source_files_properties(
    ${_WEBRTC_OBJ_PATH} PROPERTIES
    EXTERNAL_OBJECT TRUE  # Identifies this as an object file
    GENERATED TRUE  # Avoids need for file to exist at configure-time
  )

  list(APPEND WEBRTC_OBJECTS_INTERNAL_DEBUG ${_WEBRTC_OBJ_PATH})
  list(APPEND WEBRTC_OBJECTS_INTERNAL debug ${_WEBRTC_OBJ_PATH})
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
  find_library(AUDIO_TOOLBOX_LIBRARY AudioToolBox)
  find_library(CORE_GRAPHICSLIBRARY CoreGraphics)
  find_library(CORE_VIDEO_LIBRARY CoreVideo)
  find_library(QTKIT_LIBRARY QTKit)
  find_library(COCOA_LIBRARY Cocoa)

  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${FOUNDATION_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_FOUNDATION_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_SERVICES_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_AUDIO_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${AUDIO_TOOLBOX_LIBRARY})    
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_GRAPHICSLIBRARY})   
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${CORE_VIDEO_LIBRARY})
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${QTKIT_LIBRARY})
  list(APPEND WEBRTC_LIBRARIES_EXTERNAL ${COCOA_LIBRARY})
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
      -DV8_DEPRECATION_WARNINGS -DENABLE_MDNS=1 -DENABLE_NOTIFICATIONS -DENABLE_PEPPER_CDMS
      -DENABLE_PLUGINS=1 -DENABLE_PDF=1 -DENABLE_PRINTING=1 -DENABLE_BASIC_PRINTING=1
      -DENABLE_PRINT_PREVIEW=1 -DENABLE_SPELLCHECK=1 -DUSE_AURA=1 -DUSE_DEFAULT_RENDER_THEME=1
      -DNO_TCMALLOC -DENABLE_WEBRTC=1 -DENABLE_EXTENSIONS=1 -DENABLE_TASK_MANAGER=1
      -DENABLE_THEMES=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1 -DENABLE_SESSION_SERVICE=1
      -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_SUPERVISED_USERS=1 -DENABLE_SERVICE_DISCOVERY=1
      -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD -DSAFE_BROWSING_DB_LOCAL -DCHROMIUM_BUILD
      -DENABLE_MEDIA_ROUTER=1 -DFIELDTRIAL_TESTING_ENABLED -D__STD_C -D_CRT_RAND_S
      -D_CRT_SECURE_NO_DEPRECATE -D_HAS_EXCEPTIONS=0 -D_SCL_SECURE_NO_DEPRECATE -D_ATL_NO_OPENGL
      -D_WINDOWS -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS -DPSAPI_VERSION=1 -DWIN32 -D_SECURE_ATL
      -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_UNICODE -DUNICODE
      -DNVALGRIND
      -DDYNAMIC_ANNOTATIONS_ENABLED=0 -DWEBRTC_WIN -D_CRT_SECURE_NO_WARNINGS

      # Skipped from origianl peerconnection_client.ninja 
      # -D_USING_V110_SDK71_ -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000000 -DWINVER=0x0A00 -DNDEBUG 
    )
elseif (APPLE)
  # from api/peerconnection_unittests.ninja
  set(WEBRTC_DEFINES
    -DHAVE_SCTP -DV8_DEPRECATION_WARNINGS -DENABLE_NOTIFICATIONS
    -DENABLE_PEPPER_CDMS -DENABLE_PLUGINS=1 -DENABLE_PDF=1
    -DENABLE_PRINTING=1 -DENABLE_BASIC_PRINTING=1 -DENABLE_PRINT_PREVIEW=1
    -DENABLE_SPELLCHECK=1 -DUSE_BROWSER_SPELLCHECKER=1 -DNO_TCMALLOC
    -DUSE_EXTERNAL_POPUP_MENU=1 -DENABLE_WEBRTC=1 -DENABLE_EXTENSIONS=1
    -DENABLE_TASK_MANAGER=1 -DENABLE_THEMES=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1
    -DENABLE_SESSION_SERVICE=1 -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_SUPERVISED_USERS=1
    -DENABLE_SERVICE_DISCOVERY=1 -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD
    -DSAFE_BROWSING_DB_LOCAL -DCHROMIUM_BUILD -DENABLE_MEDIA_ROUTER=1
    -DFIELDTRIAL_TESTING_ENABLED
    -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D_FORTIFY_SOURCE=2
    -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORE=0 -DNVALGRIND
    -DDYNAMIC_ANNOTATIONS_ENABLED=0 -DWEBRTC_RESTRICT_LOGGING
    -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE -DEXPAT_RELATIVE_PATH -DWEBRTC_POSIX -DWEBRTC_MAC
    -DSRTP_RELATIVE_PATH -DHAVE_SCTP -DHAVE_SRTP
    )

elseif (UNIX)
  set(WEBRTC_DEFINES
    # from peerconnection_client.ninja
    -DV8_DEPRECATION_WARNINGS -DENABLE_MDNS=1 -DENABLE_NOTIFICATIONS -DENABLE_PEPPER_CDMS
    -DENABLE_PLUGINS=1 -DENABLE_PDF=1 -DENABLE_PRINTING=1 -DENABLE_BASIC_PRINTING=1
    -DENABLE_PRINT_PREVIEW=1 -DENABLE_SPELLCHECK=1 -DUSE_UDEV -DUI_COMPOSITOR_IMAGE_TRANSPORT
    -DUSE_AURA=1 -DUSE_PANGO=1 -DUSE_CAIRO=1 -DUSE_CLIPBOARD_AURAX11=1 -DUSE_DEFAULT_RENDER_THEME=1
    -DUSE_GLIB=1 -DUSE_NSS_CERTS=1 -DUSE_X11=1 -DENABLE_WEBRTC=1 -DENABLE_EXTENSIONS=1
    -DENABLE_TASK_MANAGER=1 -DENABLE_THEMES=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1
    -DENABLE_SESSION_SERVICE=1 -DENABLE_SUPERVISED_USERS=1 -DENABLE_SERVICE_DISCOVERY=1
    -DFULL_SAFE_BROWSING -DSAFE_BROWSING_CSD -DSAFE_BROWSING_DB_LOCAL -DCHROMIUM_BUILD
    -DENABLE_MEDIA_ROUTER=1 -DFIELDTRIAL_TESTING_ENABLED
    -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D__STDC_CONSTANT_MACROS
    -D__STDC_FORMAT_MACROS -DNVALGRIND -DDYNAMIC_ANNOTATIONS_ENABLED=0
    -DWEBRTC_BUILD_LIBEVENT -DWEBRTC_POSIX -DWEBRTC_LINUX
    # Skipped from origianl peerconnection_client.ninja 
    # -DNDEBUG -DCR_CLANG_REVISION=277962-1
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