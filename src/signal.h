/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_SIGNAL_H__
#define __THROUGHNET_SIGNAL_H__

#include <string>
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/sigslot.h"


namespace tn {

class SignalInterface
    : public rtc::RefCountInterface {
public:
  virtual void SignIn(std::string& url, std::string& id, std::string& password) = 0;
  virtual void Connect(std::string& channel) = 0;
  virtual void Disconnect(std::string& channel) = 0;
  virtual void SendCommand(std::string command, std::string& message) = 0;

  std::string full_id() { return full_id_; }
  std::string channel() { return channel_; }
 
  // sigslots
  sigslot::signal1<std::string&> SignalOnSignedIn_;
  sigslot::signal1<std::string&> SignalOnConnectToPeer_;
  sigslot::signal2<std::string&, std::string&> SignalOnCommandReceived_;


protected:
  std::string full_id_;
  std::string channel_;
};

class Signal
    : public SignalInterface {
public:
  virtual void SignIn(std::string& url, std::string& id, std::string& password) {};
  virtual void Connect(std::string& channel) {};
  virtual void Disconnect(std::string& channel) {};
  virtual void SendCommand(std::string command, std::string& message) {}

private:

}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__