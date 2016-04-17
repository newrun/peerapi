/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

//#include "config.h"

#include <map>
#include "signalconnection.h"
#include "webrtc/base/logging.h"


namespace tn {

Signal::Signal() : open_(false), done_(false) {
  // set up access channels to only log interesting things
  client_.clear_access_channels(websocketpp::log::alevel::all);
  client_.set_access_channels(websocketpp::log::alevel::connect);
  client_.set_access_channels(websocketpp::log::alevel::disconnect);
  client_.set_access_channels(websocketpp::log::alevel::app);

  // Initialize ASIO
  client_.init_asio();

  // Bind the handlers we are using
  using websocketpp::lib::placeholders::_1;
  using websocketpp::lib::placeholders::_2;
  using websocketpp::lib::bind;
  client_.set_message_handler(bind(&Signal::on_message, this, _1, _2));
  client_.set_open_handler(bind(&Signal::on_open, this, _1));
  client_.set_close_handler(bind(&Signal::on_close, this, _1));
  client_.set_fail_handler(bind(&Signal::on_fail, this, _1));
  client_.set_tls_init_handler(bind(&Signal::on_tls_init, this, _1));
}

void Signal::SignIn() {

  if (thread_.joinable()) {
    LOG(LS_ERROR) << "Already signning...";
    return;
  }

  // Start connection thread
  thread_ = websocketpp::lib::thread([&]() {
    // Create a new connection to the given URI
    websocketpp::lib::error_code ec;
    client::connection_ptr con = client_.get_connection(url_, ec);
    if (ec) {
      client_.get_alog().write(websocketpp::log::alevel::app,
        "Get Connection Error: " + ec.message());
      return;
    }

    // Grab a handle for this connection so we can talk to it in a thread
    // safe manor after the event loop starts.
    hdl_ = con->get_handle();

    // Queue the connection. No DNS queries or network connections will be
    // made until the io_service event loop is run.
    client_.connect(con);

    // Create a thread to run the ASIO io_service event loop
    client_.run();
  });
}


void Signal::SetConfig(const std::string& url,
                       const std::string& user_id,
                       const std::string& user_password) {
  url_ = url;
  user_id_ = user_id;
  user_password_ = user_password;
}


void Signal::SendCommand(const Json::Value& data, const std::string eventname, const std::string sio_namespace) {
  /*
  if (!sio_.opened()) return;

  std::string nsp = sio_namespace.empty() ? "/" + channel_ : sio_namespace;

  Json::FastWriter writer;
  sio_.socket(nsp)->emit(eventname, writer.write(data));
  */
  return;

}

bool Signal::SendCommand(const Json::Value& data) {
  /*
  SendCommand(data, "command", "");
  */
  return true;
  
}


/*
void Signal::on_connected() {

  session_id_ = sio_.get_sessionid();
  bind_events(sio_.socket());

  Json::Value data;

  data["user_id"] = user_id_;
  data["user_password"] = user_password_;
  data["channel"] = channel_;

  SendCommand(data, "signin", "/");

}

void Signal::on_close(sio::client::close_reason const& reason) {

}

void Signal::on_fail() {

}

void Signal::bind_events(const sio::socket::ptr &socket) {
  socket->on("signin", sio::socket::event_listener_aux([&](std::string const& name,
                                                           sio::message::ptr const& data,
                                                           bool isAck,
                                                           sio::message::list &ack_resp) {
    bool result = data->get_map()["result"]->get_bool();
    if (!result) {
      LOG(LS_ERROR) << "Failed to sign in";
    }

    SignalOnSignedIn_(session_id_);
  }));

  socket->on("command", sio::socket::event_listener_aux([&](std::string const& name,
                                                            sio::message::ptr const& data,
                                                            bool isAck,
                                                            sio::message::list &ack_resp) {

  }));
}
*/

void Signal::run() {
  /*
  // Create a new connection to the given URI
  websocketpp::lib::error_code ec;
  client::connection_ptr con = client_.get_connection(url_, ec);
  if (ec) {
    client_.get_alog().write(websocketpp::log::alevel::app,
      "Get Connection Error: " + ec.message());
    return;
  }

  // Grab a handle for this connection so we can talk to it in a thread
  // safe manor after the event loop starts.
  hdl_ = con->get_handle();

  // Queue the connection. No DNS queries or network connections will be
  // made until the io_service event loop is run.
  client_.connect(con);

  // Create a thread to run the ASIO io_service event loop
  websocketpp::lib::thread asio_thread(&client::run, &client_);

  // Create a thread to run the telemetry loop
  websocketpp::lib::thread telemetry_thread(&Signal::loop, this);

  asio_thread.join();
  telemetry_thread.join();
  */
}


void Signal::on_message(websocketpp::connection_hdl hdl, message_ptr) {
  scoped_lock guard(lock_);
}

// The open handler will signal that we are ready to start sending telemetry
void Signal::on_open(websocketpp::connection_hdl) {
  client_.get_alog().write(websocketpp::log::alevel::app,
    "Connection opened, starting signalconnection!");

  scoped_lock guard(lock_);
  open_ = true;
}

// The close handler will signal that we should stop sending telemetry
void Signal::on_close(websocketpp::connection_hdl) {
  client_.get_alog().write(websocketpp::log::alevel::app,
    "Connection closed, stopping signalconnection!");

  scoped_lock guard(lock_);
  done_ = true;
}

// The fail handler will signal that we should stop sending telemetry
void Signal::on_fail(websocketpp::connection_hdl) {
  client_.get_alog().write(websocketpp::log::alevel::app,
    "Connection failed, stopping signalconnection!");

  scoped_lock guard(lock_);
  done_ = true;
}


Signal::context_ptr Signal::on_tls_init(websocketpp::connection_hdl) {
  context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv1);

  try {
    ctx->set_options(asio::ssl::context::default_workarounds |
                     asio::ssl::context::no_sslv2 |
                     asio::ssl::context::no_sslv3 |
                     asio::ssl::context::single_dh_use);
  }
  catch (std::exception&) {
    // NOTHING
  }
  return ctx;
}


void Signal::loop() {
  /*
  uint64_t count = 0;
  std::stringstream val;
  websocketpp::lib::error_code ec;

  while (1) {
    bool wait = false;

    {
      scoped_lock guard(lock_);
      // If the connection has been closed, stop generating telemetry
      if (done_) { break; }

      // If the connection hasn't been opened yet wait a bit and retry
      if (!open_) {
        wait = true;
      }
    }

    if (wait) {
      //sleep(1);
      continue;
    }

    val.str("");
    val << "count is " << count++;

    client_.get_alog().write(websocketpp::log::alevel::app, val.str());
    client_.send(hdl_, val.str(), websocketpp::frame::opcode::text, ec);

    // The most likely error that we will get is that the connection is
    // not in the right state. Usually this means we tried to send a
    // message to a connection that was closed or in the process of
    // closing. While many errors here can be easily recovered from,
    // in this simple example, we'll stop the telemetry loop.
    if (ec) {
      client_.get_alog().write(websocketpp::log::alevel::app,
        "Send Error: " + ec.message());
      break;
    }

    //sleep(1);
  }
  */
}

} // namespace tn