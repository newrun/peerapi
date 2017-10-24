/*
*  Copyright 2016 The PeerApi Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#ifndef __PEERAPI_CONTROLOBSERVER_H__
#define __PEERAPI_CONTROLOBSERVER_H__

#include "common.h"

namespace peerapi {

class ControlObserver {
public:
  virtual void OnOpen(const std::string peer_id) = 0;
  virtual void OnClose(const std::string peer_id, const peerapi::CloseCode code, const std::string desc = "") = 0;
  virtual void OnConnect(const std::string peer_id) = 0;
  virtual void OnMessage(const std::string peer_id, const char* data, const size_t size) = 0;
  virtual void OnWritable(const std::string peer_id) = 0;
};

} // namespace peerapi

#endif // __PEERAPI_CONTROLOBSERVER_H__
