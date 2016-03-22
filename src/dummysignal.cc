/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#include "config.h"

#include "webrtc/base/helpers.h"
#include "webrtc/base/logging.h"
#include "dummysignal.h"


namespace tn {

std::map<std::string, DummySignal::PeerSignal> DummySignal::connections_;

void DummySignal::SignIn(std::string& url, std::string& id, std::string& password) {
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
    connections_[channel][0]->SignalOnConnectToPeer_(connections_[channel][1]->session_id());
  }


}

void DummySignal::Disconnect(std::string& channel) {
  if (connections_.find(channel) == connections_.end())
    return;

  connections_.erase(channel);
}

void DummySignal::SendCommand(std::string command, std::string& message) {
  if (connections_.find(channel_) == connections_.end())
    return;

  for (auto& connection : connections_[channel_]) {
    if (connection != this) {
      connection->SignalOnCommandReceived_(command, message);
    }
  }
}


} // namespace tn
