/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#ifndef __PEERCONNECT_CONTROLOBSERVER_H__
#define __PEERCONNECT_CONTROLOBSERVER_H__

#include "common.h"

namespace pc {

class ControlObserver {
public:
  virtual void OnOpen(const std::string channel) = 0;
  virtual void OnClose(const std::string channel, const pc::CloseCode code, const std::string desc = "") = 0;
  virtual void OnConnect(const std::string channel) = 0;
  virtual void OnMessage(const std::string channel, const char* buffer, const size_t size) = 0;
  virtual void OnWritable(const std::string channel) = 0;
};

} // namespace pc

#endif // __PEERCONNECT_CONTROLOBSERVER_H__
