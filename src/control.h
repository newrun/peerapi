/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#ifndef __THROUGHNET_THROUGHENT_H__
#define __THROUGHNET_THROUGHENT_H__

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/sigslot.h"

namespace tn {

class Control
  : public webrtc::PeerConnectionObserver,
  public webrtc::CreateSessionDescriptionObserver,
  public sigslot::has_slots<> {

public:
  Control() {}
  virtual ~Control() {}

  //
  // PeerConnectionObserver implementation.
  //

  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {};
  void OnAddStream(webrtc::MediaStreamInterface* stream) override {};
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override {};
  void OnDataChannel(webrtc::DataChannelInterface* channel) override {};
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {};
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {};
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {};
  void OnIceConnectionReceivingChange(bool receiving) override {}


  //
  // Implements CreateSessionDescriptionObserver.
  //

  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) {};
  virtual void OnFailure(const std::string& error) {}

protected:

};

} // namespace tn

#endif // __THROUGHNET_THROUGHENT_H__
