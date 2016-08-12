/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#ifndef __PEERCONNECT_CONTROLOBSERVER_H__
#define __PEERCONNECT_CONTROLOBSERVER_H__

namespace pc {

class ControlObserver {
public:
  virtual void OnSignedIn(const std::string id) = 0;
  virtual void OnSignedOut(const std::string id) = 0;
  virtual void OnPeerConnected(const std::string id) = 0;
  virtual void OnPeerDisconnected(const std::string id) = 0;
  virtual void OnPeerMessage(const std::string id, const char* buffer, const size_t size) = 0;
  virtual void OnPeerWritable(const std::string id) = 0;
  virtual void OnError(const std::string id, const std::string& reason) = 0;
};

} // namespace pc

#endif // __PEERCONNECT_CONTROLOBSERVER_H__
