/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

 /*
   Reference: socket.io-client-cpp

   Copyright (c) 2015, Melo Yao
   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to all conditions.

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */


#ifndef __THROUGHNET_SIGNAL_H__
#define __THROUGHNET_SIGNAL_H__

#include <string>

#if _DEBUG || DEBUG
#include <websocketpp/config/debug_asio.hpp>
#else
#include <websocketpp/config/asio_client.hpp>
#endif //DEBUG

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
  virtual void SendCommand(const std::string channel,
                           const std::string eventname,
                           const Json::Value& data) = 0;
  virtual void SendGlobalCommand(const std::string eventname,
                           const Json::Value& data) = 0;

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
  enum con_state
  {
    con_opening,
    con_opened,
    con_closing,
    con_closed
  };

#if _DEBUG || DEBUG
  typedef websocketpp::config::debug_asio_tls client_config;
#else
  typedef websocketpp::config::asio_tls_client client_config;
#endif //DEBUG
  typedef websocketpp::client<client_config> client_type;

  Signal::Signal();
  Signal::~Signal();

  virtual void SignIn();

  void SendCommand(const std::string channel,
                   const std::string eventname,
                   const Json::Value& data);
  void SendGlobalCommand(const std::string eventname,
                         const Json::Value& data);


  void SetConfig(const std::string& url,
                 const std::string& user_id,
                 const std::string& user_password);


  bool opened() const { return con_state_ == con_opened; }
  void set_reconnect_attempts(unsigned attempts) { reconn_attempts_ = attempts; }
  void set_reconnect_delay(unsigned millis) { reconn_delay_ = millis; if (reconn_delay_max_<millis) reconn_delay_max_ = millis; }
  void set_reconnect_delay_max(unsigned millis) { reconn_delay_max_ = millis; if (reconn_delay_>millis) reconn_delay_ = millis; }


protected:
  void Connect();
  void Close();
  void SyncClose();
  asio::io_service& GetIoService();

private:
  void SendSignInCommand();
  void OnSignInCommand(Json::Value& data);

  void RunLoop();
  void ConnectInternal();
  void CloseInternal(websocketpp::close::status::value const& code, std::string const& reason);
  void TimeoutReconnect(websocketpp::lib::asio::error_code const& ec);
  unsigned NextDelay() const;

  //websocket callbacks
  void OnFail(websocketpp::connection_hdl con);
  void OnOpen(websocketpp::connection_hdl con);
  void OnClose(websocketpp::connection_hdl con);
  void OnMessage(websocketpp::connection_hdl con, client_type::message_ptr msg);

  void ResetState();

  typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;
  context_ptr OnTlsInit(websocketpp::connection_hdl con);

  // Connection pointer for client functions.
  websocketpp::connection_hdl con_hdl_;
  client_type client_;

  std::unique_ptr<std::thread> network_thread_;
  std::unique_ptr<websocketpp::lib::asio::steady_timer> reconn_timer_;
  con_state con_state_;

  unsigned reconn_delay_;
  unsigned reconn_delay_max_;
  unsigned reconn_attempts_;
  unsigned reconn_made_;

  // Signin
  std::string url_;
  std::string user_id_;
  std::string user_password_;
}; // class Signal


} // namespace tn

#endif // __THROUGHNET_SIGNAL_H__