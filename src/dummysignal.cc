/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#include "webrtc/base/helpers.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/json.h"
#include "dummysignal.h"


namespace tn {

std::map<std::string, DummySignal::PeerSignal> DummySignal::connections_;

void DummySignal::SignIn() {
  session_id_ = rtc::CreateRandomUuid();
  SignalOnSignedIn_(session_id_);
}

void DummySignal::Connect(std::string& channel) {

  // First connection
  if (connections_.find(channel) == connections_.end()) {
    channel_ = channel;
    PeerSignal peer_signal;
    peer_signal.push_back(this);
    connections_.insert(std::pair<std::string, PeerSignal>(channel, peer_signal));
  }
  // Second connection
  else {
    if (connections_[channel].size() != 1) {
      LOG(LS_ERROR) << "Support only two peer in the same channel.";
      return;
    }
  
    channel_ = channel;
    connections_[channel].push_back(this);

    Json::Value jmessage;
    Json::Value data;
    jmessage["command"] = "createoffer";
    jmessage["peer_sid"] = connections_[channel][0]->session_id();
    data["peer_sid"] = connections_[channel][1]->session_id();
    jmessage["data"] = data;

    SendCommand(jmessage);
  }
}

void DummySignal::Disconnect(std::string& channel) {
  if (connections_.find(channel) == connections_.end())
    return;

  connections_.erase(channel);
}


bool DummySignal::SendCommand(const Json::Value& jmessage) {

  Json::FastWriter writer;
  Json::Value final_message;
  Json::Value data;
  std::string peer_sid;
  std::string command;

  if (!rtc::GetStringFromJsonObject(jmessage, "command", &command)) return false;
  if (!rtc::GetValueFromJsonObject(jmessage, "data", &data)) return false;

  final_message["command"] = command;
  final_message["data"] = data;

  if (rtc::GetStringFromJsonObject(jmessage, "peer_sid", &peer_sid)) {
    final_message["peer_sid"] = session_id_;
  }

  for (auto& connection : connections_[channel_]) {
    if (peer_sid.empty() || peer_sid == connection->session_id()) {
      connection->SignalOnCommandReceived_(writer.write(final_message));
    }
  }

  return true;
}

} // namespace tn
