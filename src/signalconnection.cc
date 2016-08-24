/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
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

#if defined(WEBRTC_WIN)
#pragma warning(disable:4503)
#endif

#include <map>
#include <list>
#include "signalconnection.h"
#include "logging.h"

namespace pc {

Signal::Signal(const std::string url) :
      con_state_(con_closed),
      network_thread_(),
      reconn_attempts_(0xFFFFFFFF),
      reconn_made_(0),
      reconn_delay_(5000),
      reconn_delay_max_(25000),
      url_(url) {

#if _DEBUG || DEBUG
  client_.clear_access_channels(websocketpp::log::alevel::all);
  client_.set_access_channels(websocketpp::log::alevel::fail);
#else
  client_.clear_access_channels(websocketpp::log::elevel::all);
  client_.clear_error_channels(websocketpp::log::alevel::all);
#endif

  // Default settings
  if (url_.empty()) {
    url_ = "wss://signal.throughnet.com/hello";
  }

  // Initialize ASIO
  client_.init_asio();

  // Bind the handlers we are using
  using websocketpp::lib::placeholders::_1;
  using websocketpp::lib::placeholders::_2;
  using websocketpp::lib::bind;

  client_.set_open_handler(bind(&Signal::OnOpen, this, _1));
  client_.set_close_handler(bind(&Signal::OnClose, this, _1));
  client_.set_fail_handler(bind(&Signal::OnFail, this, _1));
  client_.set_message_handler(bind(&Signal::OnMessage, this, _1, _2));
  client_.set_tls_init_handler(bind(&Signal::OnTlsInit, this, _1));

  LOGP_F( INFO ) << "Done";
}

Signal::~Signal() {
#if _DEBUG || DEBUG
  client_.clear_access_channels(websocketpp::log::alevel::all);
  client_.set_access_channels(websocketpp::log::alevel::fail);
#else
  client_.clear_access_channels(websocketpp::log::elevel::all);
  client_.clear_access_channels(websocketpp::log::alevel::all);
#endif

  Teardown();
  LOGP_F( INFO ) << "Done";
}

void Signal::SignIn(const std::string& id, const std::string& password) {
  user_id_ = id;
  user_password_ = password;
  Connect();

  LOGP_F( INFO ) << "Done";
}

void Signal::SignOut() {
  if (opened()) Close();
  LOGP_F( INFO ) << "Done";
}

void Signal::SendCommand(const std::string channel,
                         const std::string commandname,
                         const Json::Value& data) {

  if (commandname.empty()) {
    LOGP_F(WARNING) << "SendCommand with empty commandname";
    return;
  }

  if (!opened()) {
    LOGP_F(WARNING) << "Signal server is not opened";
    return;
  }

  Json::Value message;
  Json::FastWriter writer;
  message["command"] = commandname;
  message["data"] = data;
  if (!channel.empty()) message["channel"] = channel;

  LOGP_F( LS_VERBOSE ) << "message is " << message.toStyledString();

  try {
    client_.send(con_hdl_, writer.write(message), websocketpp::frame::opcode::text);
  }
  catch (websocketpp::lib::error_code& ec) {
    LOGP_F(LERROR) << "SendCommand Error: " << ec;
  }
  catch (std::exception& e) {
    LOGP_F(LERROR) << "SendCommand Error: " << e.what();
  }
  catch (...) {
    LOGP_F(LERROR) << "SendCommand Error: ";
  }

  LOGP_F( INFO ) << "Done";
}

void Signal::SendGlobalCommand(const std::string commandname,
                               const Json::Value& data) {
  SendCommand("", commandname, data);
}



void Signal::Connect()
{
  if (reconn_timer_)
  {
    reconn_timer_->cancel();
    reconn_timer_.reset();
  }

  if (network_thread_)
  {
    if (con_state_ == con_closing || con_state_ == con_closed)
    {
      //if client is closing, join to wait.
      //if client is closed, still need to join,
      //but in closed case,join will return immediately.
      network_thread_->join();
      network_thread_.reset();//defensive
    }
    else
    {
      //if we are connected, do nothing.
      return;
    }
  }

  con_state_ = con_opening;
  reconn_made_ = 0;

  this->ResetState();
  client_.get_io_service().dispatch(websocketpp::lib::bind(&Signal::ConnectInternal, this));
  network_thread_.reset(new websocketpp::lib::thread(websocketpp::lib::bind(&Signal::RunLoop, this)));
  LOGP_F( INFO ) << "Done";
}


void Signal::Close()
{
  con_state_ = con_closing;
  client_.get_io_service().dispatch(websocketpp::lib::bind(&Signal::CloseInternal,
                                    this,
                                    websocketpp::close::status::normal,
                                    "End by user"));
  LOGP_F( INFO ) << "Done";
}

void Signal::SyncClose()
{
  con_state_ = con_closing;
  client_.get_io_service().dispatch(websocketpp::lib::bind(&Signal::CloseInternal,
                                    this,
                                    websocketpp::close::status::normal,
                                    "End by user"));
  if (network_thread_)
  {
    network_thread_->join();
    network_thread_.reset();
  }
  LOGP_F( INFO ) << "Done";
}

void Signal::Teardown()
{
  // TODO: Asyncronous close with PeerConnect::Stop()
  SyncClose();
  LOGP_F( INFO ) << "Done";
}


asio::io_service& Signal::GetIoService()
{
  return client_.get_io_service();
}



void Signal::SendSignInCommand() {
  Json::Value data;

  data["user_id"] = user_id_;
  data["user_password"] = user_password_;

  SendGlobalCommand("signin", data);
}

void Signal::OnCommandReceived(Json::Value& message) {
  SignalOnCommandReceived_(message);
  return;
}

void Signal::RunLoop()
{
  client_.run();
  client_.reset();
  client_.get_alog().write(websocketpp::log::alevel::devel,
                           "run loop end");
}

void Signal::ConnectInternal()
{
    websocketpp::lib::error_code ec;
    client_type::connection_ptr con = client_.get_connection(url_, ec);
    if (ec) {
      client_.get_alog().write(websocketpp::log::alevel::app,
                              "Get Connection Error: " + ec.message());
      return;
    }

    client_.connect(con);
    return;
}



void Signal::CloseInternal(websocketpp::close::status::value const& code, std::string const& reason)
{
  LOGP_F(WARNING) << "Close by reason:" << reason;

  if (reconn_timer_)
  {
    reconn_timer_->cancel();
    reconn_timer_.reset();
  }
  if (con_hdl_.expired())
  {
    LOGP_F(LERROR) << "Error: No active session";
  }
  else
  {
    websocketpp::lib::error_code ec;
    client_.close(con_hdl_, code, reason, ec);
  }
}

void Signal::TimeoutReconnect(websocketpp::lib::asio::error_code const& ec)
{
  if (ec)
  {
    return;
  }

  if (con_state_ == con_closed)
  {
    con_state_ = con_opening;
    reconn_made_++;
    this->ResetState();
    LOGP_F(WARNING) << "Reconnecting..";
    client_.get_io_service().dispatch(websocketpp::lib::bind(&Signal::ConnectInternal, this));
  }
}

unsigned Signal::NextDelay() const
{
  //no jitter, fixed power root.
  unsigned reconn_made = std::min<unsigned>(reconn_made_, 32);//protect the pow result to be too big.
  return static_cast<unsigned>(std::min<double>(reconn_delay_ * pow(1.5, reconn_made), reconn_delay_max_));
}


void Signal::OnFail(websocketpp::connection_hdl con)
{
  con_hdl_.reset();
  con_state_ = con_closed;
  LOGP_F(LERROR) << "Connection failed.";

  if (reconn_made_<reconn_attempts_)
  {
    LOGP_F(WARNING) << "Reconnect for attempt:" << reconn_made_;
    unsigned delay = this->NextDelay();
    reconn_timer_.reset(new asio::steady_timer(client_.get_io_service()));
    websocketpp::lib::asio::error_code ec;
    reconn_timer_->expires_from_now(websocketpp::lib::asio::milliseconds(delay), ec);
    reconn_timer_->async_wait(websocketpp::lib::bind(&Signal::TimeoutReconnect, this, websocketpp::lib::placeholders::_1));
  }
}

void Signal::OnOpen(websocketpp::connection_hdl con)
{
  LOGP_F(WARNING) << "Connected.";
  con_state_ = con_opened;
  con_hdl_ = con;
  reconn_made_ = 0;

  SendSignInCommand();
}


void Signal::OnClose(websocketpp::connection_hdl con)
{
  con_state_ = con_closed;
  websocketpp::lib::error_code ec;
  websocketpp::close::status::value code = websocketpp::close::status::normal;
  client_type::connection_ptr conn_ptr = client_.get_con_from_hdl(con, ec);
  if (ec) {
    LOGP_F(LERROR) << "OnClose get conn failed" << ec;
  }
  else
  {
    code = conn_ptr->get_local_close_code();
  }

  con_hdl_.reset();

  SignalOnClosed_(code);

  if (code == websocketpp::close::status::normal)
  {
    // NOTHING
  }
  else
  {
    if (reconn_made_<reconn_attempts_)
    {
      LOGP_F(LS_WARNING) << "Reconnect for attempt:" << reconn_made_;
      unsigned delay = this->NextDelay();
      reconn_timer_.reset(new websocketpp::lib::asio::steady_timer(client_.get_io_service()));
      websocketpp::lib::asio::error_code ec;
      reconn_timer_->expires_from_now(websocketpp::lib::asio::milliseconds(delay), ec);
      reconn_timer_->async_wait(websocketpp::lib::bind(&Signal::TimeoutReconnect, this, websocketpp::lib::placeholders::_1));
      return;
    }
  }

  LOGP_F( INFO ) << "Done";

}


void Signal::OnMessage(websocketpp::connection_hdl con, client_type::message_ptr msg)
{
  Json::Reader reader;
  Json::Value jmessage;

  if (!reader.parse(msg->get_payload(), jmessage)) {
    LOGP_F(WARNING) << "Received unknown message: " << msg->get_payload();
    return;
  }

  LOGP_F( LS_VERBOSE ) << jmessage.toStyledString();
  OnCommandReceived(jmessage);
}


void Signal::ResetState()
{
  client_.reset();
}

Signal::context_ptr Signal::OnTlsInit(websocketpp::connection_hdl conn)
{
  context_ptr ctx = context_ptr(new  asio::ssl::context(asio::ssl::context::tlsv1));
  websocketpp::lib::asio::error_code ec;
  ctx->set_options(asio::ssl::context::default_workarounds |
                   asio::ssl::context::no_sslv2 |
                   asio::ssl::context::single_dh_use, ec);
  if (ec)
  {
    LOGP_F(LERROR) << "Init tls failed,reason:" << ec.message();
  }

  return ctx;
}


} // namespace pc