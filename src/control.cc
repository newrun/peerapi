/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "config.h"
#include "control.h"

#include "webrtc/api/test/fakedtlsidentitystore.h"
#include "webrtc/p2p/client/fakeportallocator.h"
#include "webrtc/api/test/mockpeerconnectionobservers.h"


namespace tn {

void Control::Control::Connect(Control* caller,
                               Control* callee) {
  caller->SignalOnIceCandidateReady.connect(
      callee, &Control::AddIceCandidate);
  callee->SignalOnIceCandidateReady.connect(
      caller, &Control::AddIceCandidate);

  caller->SignalOnSdpReady.connect(
      callee, &Control::ReceiveOfferSdp);
  callee->SignalOnSdpReady.connect(
      caller, &Control::ReceiveAnswerSdp);
}
  

Control::Control(const std::string& name)
       : name_(name) {}


bool Control::CreatePc(
    const webrtc::MediaConstraintsInterface* constraints) {

  rtc::scoped_ptr<cricket::PortAllocator> port_allocator(
    new cricket::FakePortAllocator(rtc::Thread::Current(), nullptr));

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
      rtc::Thread::Current(), rtc::Thread::Current(),
      NULL, NULL, NULL);

  if (!peer_connection_factory_) {
    return false;
  }

  // CreatePeerConnection with RTCConfiguration.
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  webrtc::PeerConnectionInterface::IceServer ice_server;
  ice_server.uri = "stun:stun.l.google.com:19302";
  config.servers.push_back(ice_server);

  rtc::scoped_ptr<webrtc::DtlsIdentityStoreInterface> dtls_identity_store(
      rtc::SSLStreamAdapter::HaveDtlsSrtp() ?
      new FakeDtlsIdentityStore() : nullptr);
  
  peer_connection_ = peer_connection_factory_->CreatePeerConnection(
      config, constraints, std::move(port_allocator),
      std::move(dtls_identity_store), this);

  return peer_connection_.get() != NULL;
}


void Control::CreateOffer(
    const webrtc::MediaConstraintsInterface* constraints) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << name_
               << ": CreateOffer.";
  peer_connection_->CreateOffer(this, constraints);
}

void Control::CreateAnswer(
    const webrtc::MediaConstraintsInterface* constraints) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << name_
               << ": CreateAnswer.";
  peer_connection_->CreateAnswer(this, constraints);
}

void Control::ReceiveOfferSdp(const std::string& sdp) {
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kOffer, sdp);
  CreateAnswer(NULL);
}

void Control::ReceiveAnswerSdp(const std::string& sdp) {
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kAnswer, sdp);
}

void Control::AddIceCandidate(const std::string& sdp_mid,
                              int sdp_mline_index,
                              const std::string& candidate) {
  rtc::scoped_ptr<webrtc::IceCandidateInterface> owned_candidate(
    webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, NULL));
  
  peer_connection_->AddIceCandidate(owned_candidate.get());
}


void Control::SetLocalDescription(const std::string& type,
                                  const std::string& sdp) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << name_
               << ": SetLocalDescription " << type << " " << sdp;

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
      observer(new rtc::RefCountedObject<
                   webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetLocalDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

void Control::SetRemoteDescription(const std::string& type,
                                   const std::string& sdp) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << name_
               << ": SetRemoteDescription " << type << " " << sdp;

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetRemoteDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

} // namespace tn
