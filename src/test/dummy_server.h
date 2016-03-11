#ifndef __THROUGHNET_TEST_DUMMY_SERVER_H__
#define __THROUGHNET_TEST_DUMMY_SERVER_H__

#include "config.h"


class DummySignal :
  public Signal {
public:
  void Connect(const std::string& url);
  void OnServerConnected();
  void OnSingnIn();

protected:
};

class DummySignalServer {
public:
  static void Connect(DummySignal* signal);
  static void Signin(DummySignal* signal);
  static void Disconnect(DummySignal* signal);

protected:

};

#endif // __THROUGHNET_TEST_DUMMY_SERVER_H__
