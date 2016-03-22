/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#ifndef __THROUGHNET_CONTROL_H__
#define __THROUGHNET_CONTROL_H__

#include "webrtc/api/datachannelinterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/sigslot.h"
#include "fakeaudiocapturemodule.h"

#include "peer.h"
#include "signal.h"


#define WAIT_(ex, timeout)                     \
  do {                                              \
    uint32_t start = rtc::Time();                   \
    while (!(ex) && rtc::Time() < start + timeout) { \
      rtc::Thread::Current()->ProcessMessages(1);   \
    }                                               \
  } while (0)


namespace tn {

class Control
    : public webrtc::PeerConnectionObserver,
      public webrtc::CreateSessionDescriptionObserver,
      public sigslot::has_slots<> {

public:

  typedef std::vector<rtc::scoped_ptr<PeerDataChannelObserver> >
            DataChannelList;

  static void Connect(Control* caller,
                      Control* callee);

  explicit Control(const std::string channel);
  explicit Control(const std::string channel,
                   rtc::scoped_refptr<Signal> signal);
  ~Control();

  bool InitializePeerConnection();
  void DeletePeerConnection();
  bool Send(const std::string& message);

  void SignIn();
  void OnSignedIn(std::string& sid);
  void OnConnectToPeer(std::string& sid);
  void OnCommandReceived(std::string& command, std::string& message);
  void OnPeerOpened(std::string& peer_sid);
  void OnPeerMessage(const webrtc::DataBuffer& buffer);



  //
  // PeerConnectionObserver implementation.
  //

  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {};
  void OnAddStream(webrtc::MediaStreamInterface* stream) override {};
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override {};
  void OnDataChannel(webrtc::DataChannelInterface* channel) override;
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {};
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {};
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnIceConnectionReceivingChange(bool receiving) override {}


  //
  // Implements CreateSessionDescriptionObserver.
  //

  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
  void OnFailure(const std::string& error) {}


  void CreateOffer(const webrtc::MediaConstraintsInterface* constraints);
  void CreateAnswer(const webrtc::MediaConstraintsInterface* constraints);
  void ReceiveOfferSdp(const std::string& sdp);
  void ReceiveAnswerSdp(const std::string& sdp);
//  void AddIceCandidate(const std::string& sdp_mid, int sdp_mline_index,
//       const std::string& candidate);
  void AddIceCandidate(const std::string& message);
  void TestWaitForConnection(uint32_t kMaxWait);
  void TestWaitForChannelOpen(uint32_t kMaxWait);
  void TestWaitForMessage(const std::string& message, uint32_t kMaxWait);
  void TestWaitForClose(uint32_t kMaxWait);
  bool CheckForConnection();


  // sigslots
  sigslot::signal1<std::string&> SignalOnConnected_;
  sigslot::signal3<const std::string&, const char*, const size_t> SignalOnData_;
//  sigslot::signal1<std::string*> SignalOnIceCandidateCreated;
//  sigslot::signal3<const std::string&,
//                   int,
//                   const std::string&> SignalOnIceCandidateReady;
//  sigslot::signal1<std::string*> SignalOnSdpCreated;
//  sigslot::signal1<const std::string&> SignalOnSdpReady;
//  sigslot::signal1<webrtc::DataChannelInterface*> SignalOnDataChannel;

protected:
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);
  bool CreatePeerConnection(const webrtc::MediaConstraintsInterface* constraints);
  bool CreateDataChannel(const std::string& label,
       const webrtc::DataChannelInit& init);

  void SetLocalDescription(const std::string& type, const std::string& sdp);
  void SetRemoteDescription(const std::string& type, const std::string& sdp);

  std::string channel_name_;
  std::string device_id_;
  std::string session_id_;
  rtc::scoped_refptr<Signal> signal_;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;
  rtc::scoped_ptr<PeerDataChannelObserver> local_data_channel_;
  rtc::scoped_ptr<PeerDataChannelObserver> remote_data_channel_;
};

} // namespace tn

#endif // __THROUGHNET_CONTROL_H__
