/*
 *  Copyright 2016 The PeerApi Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#ifndef __PEERAPI_PEERCONNECT_H__
#define __PEERAPI_PEERCONNECT_H__

#include <string>
#include <map>
#include <memory>
#include <functional>

#include "common.h"
#include "controlobserver.h"

#ifndef USE_PEERAPI_STRICT_NAMESPACE
using namespace peerapi;
#endif // USE_PEERAPI_STRICT_NAMESPACE

namespace peerapi {

class Control;
class Signal;


class Peer
  : public ControlObserver {
public:

  using string  = std::string;
  using Data    = std::map<string, string>;

  struct Setting {
    string signal_uri_;
    string signal_id_;
    string signal_password_;
  };

  //
  // APIs
  //

  static void Run();
  static void Stop();

  void Open();
  void Close( const string peer_id = "" );
  void Connect( const string peer_id );
  bool Send( const string& peer_id, const char* data, const std::size_t size, const bool wait = SYNC_OFF );
  bool Send( const string& peer_id, const string& data, const bool wait = SYNC_OFF );
  bool SetOptions( const string options );

  Peer& On( string event_id, std::function<void( string )> );
  Peer& On( string event_id, std::function<void( string, string )> );
  Peer& On( string event_id, std::function<void( string, peerapi::CloseCode, string )> );
  Peer& On( string event_id, std::function<void( string, char*, std::size_t )> );

  //
  // Member functions
  //

  explicit Peer( const string peer_id = "" );
  ~Peer();

  static std::string CreateRandomUuid();


protected:
  // The base type that is stored in the collection.
  struct Handler_t {
    virtual ~Handler_t() = default;
  };

  // The derived type that represents a callback.
  template<typename ...A>
  struct EventHandler_t : public Handler_t {
    using cb = std::function<void( A... )>;
    cb callback_;
    EventHandler_t( cb p_callback ) : callback_( p_callback ) {}
  };

  template<typename ...A>
  void CallEventHandler( string msg_id, A&& ... args );

  using EventHandler_1 = EventHandler_t<>;
  using EventHandler_2 = EventHandler_t<string>;
  using EventHandler_3 = EventHandler_t<string, Data&>;
  using EventHandler_Close = EventHandler_t<string, peerapi::CloseCode, string>;
  using EventHandler_Message = EventHandler_t<string, char*, std::size_t>;
  using Events = std::map<string, std::unique_ptr<Handler_t>>;

  //
  // ControlObserver implementation
  //

  void OnOpen( const string peer_id );
  void OnClose( const string peer_id, const peerapi::CloseCode code, const string desc = "" );
  void OnConnect( const string peer_id );
  void OnMessage( const string peer_id, const char* data, const size_t size );
  void OnWritable( const string peer_id );

  bool ParseOptions( const string& options );

  bool close_once_;
  Setting setting_;
  Events event_handler_;

  std::shared_ptr<Control> control_;
  std::shared_ptr<Signal> signal_;

  string peer_id_;
};


} // namespace peerapi

#endif // __PEERAPI_PEERCONNECT_H__
