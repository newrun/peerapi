/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "config.h"
#include "control.h"
#include "peer.h"

#include "webrtc/api/test/fakedtlsidentitystore.h"
#include "webrtc/p2p/client/fakeportallocator.h"
#include "webrtc/api/test/mockpeerconnectionobservers.h"
#include "webrtc/base/json.h"


namespace tn {

void Control::Control::Connect(Control* caller,
                               Control* callee) {
//  caller->SignalOnIceCandidateReady.connect(
//      callee, &Control::AddIceCandidate);
//  callee->SignalOnIceCandidateReady.connect(
//      caller, &Control::AddIceCandidate);

//  caller->SignalOnSdpReady.connect(
//      callee, &Control::ReceiveOfferSdp);
//  callee->SignalOnSdpReady.connect(
//      caller, &Control::ReceiveAnswerSdp);
}

  

Control::Control(const std::string channel)
       : Control(channel, nullptr){
}

Control::Control(const std::string channel, rtc::scoped_refptr<Signal> signal)
       : channel_name_(channel),
         signal_(signal) {

  signal_->SignalOnSignedIn_.connect(this, &Control::OnSignedIn);
  signal_->SignalOnConnectToPeer_.connect(this, &Control::OnConnectToPeer);
  signal_->SignalOnCommandReceived_.connect(this, &Control::OnCommandReceived);
}

Control::~Control() {
  DeletePeerConnection();
}


bool Control::InitializePeerConnection() {

  ASSERT(peer_connection_factory_.get() == NULL);
  ASSERT(peer_connection_.get() == NULL);

  webrtc::MediaConstraintsInterface* constraints = NULL;

  if (!CreatePeerFactory(constraints)) {
    LOG(LS_ERROR) << "CreatePeerFactory failed";
    DeletePeerConnection();
    return false;
  }

  if (!CreatePeerConnection(constraints)) {
    LOG(LS_ERROR) << "CreatePeerConnection failed";
    DeletePeerConnection();
    return false;
  }

  webrtc::DataChannelInit init;
  const std::string data_channel_name = std::string("tn_data_") + session_id_;
  if (!CreateDataChannel(data_channel_name, init)) {
    LOG(LS_ERROR) << "CreateDataChannel failed";
    DeletePeerConnection();
  }

  return true;
}

void Control::DeletePeerConnection() {
  local_data_channel_ = NULL;
  remote_data_channel_ = NULL;
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;
  fake_audio_capture_module_ = NULL;
}

bool Control::CreatePeerFactory(
    const webrtc::MediaConstraintsInterface* constraints) {

  fake_audio_capture_module_ = FakeAudioCaptureModule::Create();
  if (fake_audio_capture_module_ == NULL) {
    return false;
  }

  peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
    rtc::Thread::Current(), rtc::Thread::Current(),
    fake_audio_capture_module_, NULL, NULL);

  if (!peer_connection_factory_.get()) {
    return false;
  }

  return true;
}

bool Control::Send(const std::string& message) {
  return local_data_channel_->Send(message);
}

void Control::SignIn() {
  if (signal_.get() == NULL) {
    LOG(LS_ERROR) << "SignIn failed, no signal server";
    return;
  }

  signal_->SignIn(std::string(""), std::string(""), std::string(""));
  return;
}

void Control::OnSignedIn(std::string& full_id) {
  signal_->Connect(channel_name_);
}

void Control::OnConnectToPeer(std::string& full_id) {
  CreateOffer(NULL);
}

void Control::OnCommandReceived(std::string& command, std::string& message) {

  if (command == "offersdp") {
    ReceiveOfferSdp(message);
  }
  else if (command == "answersdp") {
    ReceiveAnswerSdp(message);
  }
  else if (command == "ice_candidate_ready") {
    AddIceCandidate(message);
  }

}

void Control::OnPeerOpened(std::string& peer_id) {
  if (local_data_channel_.get() != nullptr && remote_data_channel_.get() != nullptr &&
      local_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen &&
      remote_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen
    ) {
    SignalOnConnected_(std::string(""));
  }
}


bool Control::CreatePeerConnection(
      const webrtc::MediaConstraintsInterface* constraints) {
  ASSERT(peer_connection_factory_.get() != NULL);
  ASSERT(peer_connection_.get() == NULL);

  rtc::scoped_ptr<cricket::PortAllocator> port_allocator(
    new cricket::FakePortAllocator(rtc::Thread::Current(), nullptr));


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


bool
Control::CreateDataChannel(
    const std::string& label,
    const webrtc::DataChannelInit& init) {

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;

  data_channel = peer_connection_->CreateDataChannel(label, &init);
  if (data_channel.get() == NULL) {
    return false;
  }

  local_data_channel_.reset(new PeerDataChannelObserver(data_channel));
  if (local_data_channel_.get() == NULL) {
    return false;
  }

  local_data_channel_->SignalOnOpen_.connect(this, &Control::OnPeerOpened);
  return true;
}

void Control::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  PeerDataChannelObserver* Observer = new PeerDataChannelObserver(data_channel);
  remote_data_channel_ = rtc::scoped_ptr<PeerDataChannelObserver>(Observer);
  remote_data_channel_->SignalOnOpen_.connect(this, &Control::OnPeerOpened);
//  SignalOnDataChannel(data_channel);
}



void Control::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string sdp;
  
  if (!candidate->ToString(&sdp)) {
    return;
  }

  // Give the user a chance to modify sdp for testing.
//  SignalOnIceCandidateCreated(&sdp);

  Json::StyledWriter writer;
  Json::Value jmessage;

  jmessage["sdp_mid"] = candidate->sdp_mid();
  jmessage["sdp_mline_index"] = candidate->sdp_mline_index();
  jmessage["candidate"] = sdp;

  signal_->SendCommand("ice_candidate_ready", writer.write(jmessage));
///  SignalOnIceCandidateReady(candidate->sdp_mid(), candidate->sdp_mline_index(),
///    sdp);
}

void Control::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  // This callback should take the ownership of |desc|.
  rtc::scoped_ptr<webrtc::SessionDescriptionInterface> owned_desc(desc);
  std::string sdp;

  if (!desc->ToString(&sdp)) {
    return;
  }
  
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
               << ": " << desc->type() << " sdp created: " << sdp;

  // Give the user a chance to modify sdp for testing.
//  SignalOnSdpCreated(&sdp);

  SetLocalDescription(desc->type(), sdp);

  if (desc->type() == webrtc::SessionDescriptionInterface::kOffer) {
    signal_->SendCommand("offersdp", sdp);
  }
  else if (desc->type() == webrtc::SessionDescriptionInterface::kAnswer) {
    signal_->SendCommand("answersdp", sdp);
  }
//  SignalOnSdpReady(sdp);
}

void Control::CreateOffer(
    const webrtc::MediaConstraintsInterface* constraints) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
               << ": CreateOffer.";
  peer_connection_->CreateOffer(this, constraints);
}

void Control::CreateAnswer(
    const webrtc::MediaConstraintsInterface* constraints) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
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

void Control::TestWaitForConnection(uint32_t kMaxWait) {
  // Test code
  WAIT_(CheckForConnection(), kMaxWait);
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
    << ": Connected.";
}

void Control::TestWaitForChannelOpen(uint32_t kMaxWait) {
  // Test code
  WAIT_(local_data_channel_->IsOpen(), kMaxWait);
  WAIT_(remote_data_channel_->IsOpen(), kMaxWait);
}

void Control::TestWaitForMessage(const std::string& message, uint32_t kMaxWait) {
  WAIT_(message == remote_data_channel_->last_message(), kMaxWait);
}

void Control::TestWaitForClose(uint32_t kMaxWait) {
  local_data_channel_->Close();
  WAIT_(local_data_channel_->state() == webrtc::DataChannelInterface::DataState::kClosed, kMaxWait);
}


bool Control::CheckForConnection() {
  return (peer_connection_->ice_connection_state() ==
          webrtc::PeerConnectionInterface::kIceConnectionConnected) ||
         (peer_connection_->ice_connection_state() ==
          webrtc::PeerConnectionInterface::kIceConnectionCompleted);
}

void Control::AddIceCandidate(const std::string& message) {

  std::string sdp_mid;
  int sdp_mline_index;
  std::string candidate;

  Json::Reader reader;
  Json::Value jmessage;
  if (!reader.parse(message, jmessage)) {
    LOG(WARNING) << "Received unknown message. " << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(jmessage, "sdp_mid", &sdp_mid) ||
    !rtc::GetIntFromJsonObject(jmessage, "sdp_mline_index", &sdp_mline_index) ||
    !rtc::GetStringFromJsonObject(jmessage, "candidate", &candidate)) {

    LOG(LS_ERROR) << "Invalid ice_candidate_ready message";
    return;
  }

  rtc::scoped_ptr<webrtc::IceCandidateInterface> owned_candidate(
      webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, NULL));
  
  peer_connection_->AddIceCandidate(owned_candidate.get());
}


void Control::SetLocalDescription(const std::string& type,
                                  const std::string& sdp) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
               << ": SetLocalDescription " << type << " " << sdp;

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
                   webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetLocalDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

void Control::SetRemoteDescription(const std::string& type,
                                   const std::string& sdp) {
  LOG(LS_INFO) << "PeerConnectionTestWrapper " << device_id_
               << ": SetRemoteDescription " << type << " " << sdp;

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetRemoteDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

} // namespace tn
