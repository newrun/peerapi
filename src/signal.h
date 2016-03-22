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
#include "webrtc/base/json.h"


namespace tn {

class SignalInterface
    : public rtc::RefCountInterface {
public:
  virtual void SignIn(std::string& url, std::string& id, std::string& password) = 0;
  virtual void Connect(std::string& channel) = 0;
  virtual void Disconnect(std::string& channel) = 0;
  virtual bool SendCommand(const Json::Value& jmessage) = 0;

  std::string session_id() { return session_id_; }
  std::string channel() { return channel_; }
 
  // sigslots
  sigslot::signal1<const std::string&> SignalOnSignedIn_;
  sigslot::signal1<const std::string&> SignalOnCommandReceived_;


protected:
  std::string session_id_;
  std::string channel_;
};

class Signal
    : public SignalInterface {
public:
  virtual void SignIn(std::string& url, std::string& id, std::string& password) {};
  virtual void Connect(std::string& channel) {};
  virtual void Disconnect(std::string& channel) {};
  virtual bool SendCommand(const Json::Value& jmessage) { return false; };

private:

}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__