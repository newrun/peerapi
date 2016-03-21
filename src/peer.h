/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#ifndef __THROUGHNET_PEER_H__
#define __THROUGHNET_PEER_H__

#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/api/datachannelinterface.h"

namespace tn {

class PeerDataChannelObserver : public webrtc::DataChannelObserver {
public:
  explicit PeerDataChannelObserver(webrtc::DataChannelInterface* channel);
  virtual ~PeerDataChannelObserver();

  void OnBufferedAmountChange(uint64_t previous_amount) override;

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  bool Send(const std::string& message);
  void Close();
  bool IsOpen() const;
  const webrtc::DataChannelInterface::DataState state() const;
  const std::string& last_message() const;
  size_t received_message_count() const;

  // sigslots
  sigslot::signal1<std::string&> SignalOnOpen_;


protected:

private:
  rtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
  webrtc::DataChannelInterface::DataState state_;
  std::string last_message_;
  size_t received_message_count_;
};

} // namespace tn

#endif // __THROUGHNET_PEER_H__