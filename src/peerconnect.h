/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#ifndef __PEERCONNECT_PEERCONNECT_H__
#define __PEERCONNECT_PEERCONNECT_H__

#include <string>
#include <map>
#include <memory>
#include <functional>

#include "common.h"
#include "controlobserver.h"

namespace pc {

class Control;
class Signal;


class PeerConnect
  : public ControlObserver {
public:

  using string  = std::string;
  using Data    = std::map<string, string>;
  using Control = Control;
  using Signal  = Signal;

  struct Setting {
    string signal_uri_;
    string signal_id_;
    string signal_password_;
  };

  class Buffer {
  public:
    Buffer() : buf_( nullptr ), size_( 0 ) {};
    Buffer( const char* buf, const size_t size ) : buf_( buf ), size_( size ) {}
    const char* buf_;
    const size_t size_;
  };

  //
  // APIs
  //

  static void Run();
  static void Stop();

  void Open();
  void Close( const string channel = "" );
  void Connect( const string channel );
  bool Send( const string& channel, const char* buffer, const size_t size, const bool wait = WAITING_OFF );
  bool Send( const string& channel, const char* buffer, const bool wait = WAITING_OFF );
  bool Send( const string& channel, const string& message, const bool wait = WAITING_OFF );
  bool SetOptions( const string options );

  PeerConnect& On( string event_id, std::function<void( string )> );
  PeerConnect& On( string event_id, std::function<void( string, string )> );
  PeerConnect& On( string event_id, std::function<void( string, pc::CloseCode, string )> );
  PeerConnect& On( string event_id, std::function<void( string, Buffer& )> );

  //
  // Member functions
  //

  explicit PeerConnect( const string channel = "" );
  ~PeerConnect();

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
  using EventHandler_Close = EventHandler_t<string, pc::CloseCode, string>;
  using EventHandler_Message = EventHandler_t<string, Buffer>;
  using Events = std::map<string, std::unique_ptr<Handler_t>>;
  using MessageHandler = std::function<void( PeerConnect*, string, Buffer& )>;

  //
  // ControlObserver implementation
  //

  void OnOpen( const string channel );
  void OnClose( const string channel, const pc::CloseCode code, const string desc = "" );
  void OnConnect( const string channel );
  void OnMessage( const string channel, const char* buffer, const size_t size );
  void OnWritable( const string channel );

  bool ParseOptions( const string& options );

  bool close_once_;
  Setting setting_;
  Events event_handler_;

  std::shared_ptr<Control> control_;
  std::shared_ptr<Signal> signal_;

  string channel_;
};


} // namespace pc

#endif // __PEERCONNECT_PEERCONNECT_H__
