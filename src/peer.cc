/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "control.h"
#include "peer.h"

namespace tn {

PeerDataChannelObserver::PeerDataChannelObserver(webrtc::DataChannelInterface* channel)
  : channel_(channel), received_message_count_(0) {
  channel_->RegisterObserver(this);
  state_ = channel_->state();
}

PeerDataChannelObserver::~PeerDataChannelObserver() {
  channel_->Close();
  channel_->UnregisterObserver();
}

void PeerDataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount) {

}

void PeerDataChannelObserver::OnStateChange() {
  state_ = channel_->state();
}

void PeerDataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  last_message_.assign(buffer.data.data<char>(), buffer.data.size());
  ++received_message_count_;
}

bool PeerDataChannelObserver::Send(const std::string& message) {
  webrtc::DataBuffer buffer(message);
  return channel_->Send(buffer);
}

void PeerDataChannelObserver::Close() {
  channel_->Close();
}


bool PeerDataChannelObserver::IsOpen() const {
  return state_ == webrtc::DataChannelInterface::kOpen;
}

const webrtc::DataChannelInterface::DataState
PeerDataChannelObserver::state() const {
  return channel_->state();
}

const std::string& PeerDataChannelObserver::last_message() const {
  return last_message_;
}

size_t PeerDataChannelObserver::received_message_count() const {
  return received_message_count_;
}

} // namespace tn