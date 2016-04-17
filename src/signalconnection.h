/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#ifndef __THROUGHNET_SIGNAL_H__
#define __THROUGHNET_SIGNAL_H__

#include <string>

#include "websocketpp/config/asio_client.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp/common/thread.hpp"

#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/sigslot.h"
#include "webrtc/base/json.h"


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
  typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
  typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;
  typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
  typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;

  Signal::Signal();

  virtual void SignIn();
  virtual void Connect(std::string& channel) {};
  virtual void Disconnect(std::string& channel) {};
  
  virtual bool SendCommand(const Json::Value& data);
  virtual void SendCommand(const Json::Value& data,
                           const std::string sio_eventname,
                           const std::string sio_namespace);

  void SetConfig(const std::string& url,
                 const std::string& user_id,
                 const std::string& user_password);

protected:
  void on_open(websocketpp::connection_hdl);
  void on_close(websocketpp::connection_hdl);
  void on_fail(websocketpp::connection_hdl);
  void on_message(websocketpp::connection_hdl hdl, message_ptr);
  context_ptr on_tls_init(websocketpp::connection_hdl);
  void run();
  void Signal::loop();

private:
  client client_;
  websocketpp::lib::thread thread_;
  websocketpp::connection_hdl hdl_;
  websocketpp::lib::mutex lock_;
  bool open_;
  bool done_;

  std::string url_;
  std::string user_id_;
  std::string user_password_;
}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__