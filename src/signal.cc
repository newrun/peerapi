/*
 *  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
 *
 *  Ryan Lee (ryan.lee at throughnet.com)
 */

#include "config.h"
#include "signal.h"


namespace tn {

Signal::Signal() {
  sio_.set_open_listener(std::bind(&Signal::on_connected, this));
  sio_.set_close_listener(std::bind(&Signal::on_close, this, std::placeholders::_1));
  sio_.set_fail_listener(std::bind(&Signal::on_fail, this));
}

void Signal::SignIn() {
  sio_.connect(url_);
}


void Signal::SetConfig(const std::string& url,
                       const std::string& user_id,
                       const std::string& user_password) {
  url_ = url;
  user_id_ = user_id;
  user_password_ = user_password;
}



void Signal::on_connected() {
  int tets = 0;
}

void Signal::on_close(sio::client::close_reason const& reason) {
  int test = 0;
}

void Signal::on_fail() {
  int test = 0;
}



} // namespace tn