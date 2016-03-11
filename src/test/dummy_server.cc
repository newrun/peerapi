#include "config.h"

#include "signal.h"
#include "dummy_server.h"

static std::list<DummySignal*> clients_;


//
// class DummySignalServer implementation
//

void DummySignalServer::Connect (DummySignal* signal) {
  clients_.push_back(signal);
  signal->OnServerConnected();
}

void DummySignalServer::Signin(DummySignal* signal) {
  signal->OnSingnIn();
}

void DummySignalServer::Disconnect(DummySignal* signal) {
  clients_.remove(signal);
}



//
// class DummySignal implementation
//

void DummySignal::Connect(const std::string& url) {
  DummySignalServer::Connect(this);
}

void DummySignal::OnServerConnected() {
  DummySignalServer::Signin(this);
}

void DummySignal::OnSingnIn() {
  callback_->OnConnected();
}
