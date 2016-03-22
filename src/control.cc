/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "config.h"
#include "control.h"
#include "peer.h"

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
  signal_->SignalOnOfferPeer_.connect(this, &Control::OnOfferPeer);
  signal_->SignalOnAnswerPeer_.connect(this, &Control::OnAnswerPeer);
  signal_->SignalOnCommandReceived_.connect(this, &Control::OnCommandReceived);
}

Control::~Control() {
  peers_.clear();
}


bool Control::InitializeControl() {

  ASSERT(peer_connection_factory_.get() == NULL);

  webrtc::MediaConstraintsInterface* constraints = NULL;

  if (!CreatePeerFactory(constraints)) {
    LOG(LS_ERROR) << "CreatePeerFactory failed";
    DeleteControl();
    return false;
  }

  return true;
}

void Control::DeleteControl() {
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

bool Control::Send(const std::string& message, const std::string *peer_id) {
  bool ret = true;

  typedef std::map<std::string, rtc::scoped_refptr<PeerControl>>::iterator it_type;
  for (it_type iterator = peers_.begin(); iterator != peers_.end(); iterator++) {
    if (peer_id == nullptr || iterator->second->remote_session_id() == *peer_id) {
      if (!iterator->second->Send(message)) {
        ret = false;
      }
    }
  }

  return ret;
}


bool Control::SendCommand(const std::string& command, const Json::Value& data, const std::string& peer_sid) {
  Json::Value jmessage;

  jmessage["command"] = command;
  jmessage["data"] = data;

  if (!peer_sid.empty()) {
    jmessage["peer_sid"] = peer_sid;
  }

  return signal_->SendCommand(jmessage);
}

void Control::OnConnected(const std::string peer_id) {
  SignalOnConnected_(channel_name_, peer_id);
}

void Control::OnData(const std::string& peer_id, const char* buffer, const size_t size) {
  SignalOnData_(channel_name_, peer_id, buffer, size);
}



void Control::SignIn() {
  if (signal_.get() == NULL) {
    LOG(LS_ERROR) << "SignIn failed, no signal server";
    return;
  }

  signal_->SignIn(std::string(""), std::string(""), std::string(""));
  return;
}

void Control::OnSignedIn(const std::string& sid) {
  signal_->Connect(channel_name_);
}

void Control::OnOfferPeer(const std::string& peer_sid) {

  Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, peer_sid, this, peer_connection_factory_);
  peers_.insert(std::pair<std::string, Peer>(peer_sid, peer));

  peer->CreateOffer(NULL);
}

void Control::OnAnswerPeer(const std::string& peer_sid) {
  if (peers_.find(peer_sid) == peers_.end()) return;

  peers_[peer_sid]->CreateAnswer(NULL);
}

void Control::OnCommandReceived(const std::string& message) {

  Json::Reader reader;
  Json::Value jmessage;
  Json::Value data;
  std::string command;
  std::string peer_sid;

  if (!reader.parse(message, jmessage)) {
    LOG(WARNING) << "Received unknown message: " << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(jmessage, "command", &command) ||
      !rtc::GetValueFromJsonObject(jmessage, "data", &data)) {

    LOG(LS_ERROR) << "Invalid message:" << message;
    return;
  }

  if (!rtc::GetStringFromJsonObject(jmessage, "peer_sid", &peer_sid)) {
    peer_sid.clear();
  }

  if (command == "offersdp") {
    ReceiveOfferSdp(peer_sid, data);
  }
  else if (command == "answersdp") {
    ReceiveAnswerSdp(peer_sid, data);
  }
  else if (command == "ice_candidate") {
    AddIceCandidate(peer_sid, data);
  }

}


void Control::AddIceCandidate(const std::string& peer_sid, const Json::Value& data) {

  std::string sdp_mid;
  int sdp_mline_index;
  std::string candidate;

  if (!rtc::GetStringFromJsonObject(data, "sdp_mid", &sdp_mid)) return;
  if (!rtc::GetIntFromJsonObject(data, "sdp_mline_index", &sdp_mline_index)) return;
  if (!rtc::GetStringFromJsonObject(data, "candidate", &candidate)) return;

  if (peers_.find(peer_sid) == peers_.end()) return;
  peers_[peer_sid]->AddIceCandidate(sdp_mid, sdp_mline_index, candidate);
}

void Control::ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data) {
  std::string sdp;

  if (!rtc::GetStringFromJsonObject(data, "sdp", &sdp)) return;

  Peer peer = new rtc::RefCountedObject<PeerControl>(session_id_, peer_sid, this, peer_connection_factory_);
  peers_.insert(std::pair<std::string, Peer>(peer_sid, peer));

  peer->ReceiveOfferSdp(sdp);
}

void Control::ReceiveAnswerSdp(const std::string& peer_sid, const Json::Value& data) {
  std::string sdp;

  if (!rtc::GetStringFromJsonObject(data, "sdp", &sdp)) return;
  if (peers_.find(peer_sid) == peers_.end()) return;

  peers_[peer_sid]->ReceiveAnswerSdp(sdp);
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
  return false;
  //return (peer_connection_->ice_connection_state() ==
  //        webrtc::PeerConnectionInterface::kIceConnectionConnected) ||
  //       (peer_connection_->ice_connection_state() ==
  //        webrtc::PeerConnectionInterface::kIceConnectionCompleted);
}



} // namespace tn
