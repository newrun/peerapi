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

#include "sio_client.h"


namespace tn {

class SignalInterface
    : public rtc::RefCountInterface {
public:
  virtual void SignIn() = 0;
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
  Signal::Signal();
  virtual void SignIn();
  virtual void Connect(std::string& channel) {};
  virtual void Disconnect(std::string& channel) {};
  virtual bool SendCommand(const Json::Value& jmessage) { return false; };

  void SetConfig(const std::string& url,
                 const std::string& user_id,
                 const std::string& user_password);

protected:

  void on_connected();
  void on_close(sio::client::close_reason const& reason);
  void on_fail();

private:
  sio::client sio_;

  std::string url_;
  std::string user_id_;
  std::string user_password_;
}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__