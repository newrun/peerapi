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

#include "controlobserver.h"

#define function_pc [&]

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

  void SignIn( const string alias = "", const string id = "", const string password = "" );
  void SignOut();
  void Connect( const string id );
  void Disconnect( const string id );
  void Send( const string& id, const char* buffer, const size_t size );
  void Send( const string& id, const char* buffer );
  void Send( const string& id, const string& message );
  bool SyncSend( const string& id, const char* buffer, const size_t size );
  bool SyncSend( const string& id, const char* buffer );
  bool SyncSend( const string& id, const string& message );
  string GetErrorMessage() { return error_reason_; }

  static std::string CreateRandomUuid();

  PeerConnect& On( string event_id, std::function<void( PeerConnect*, string )> );
  PeerConnect& OnMessage( std::function<void( PeerConnect*, string, Buffer& )> );


  //
  // Member functions
  //

  explicit PeerConnect();
  explicit PeerConnect( string options );
  ~PeerConnect();


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

  using EventHandler_1 = EventHandler_t<PeerConnect*>;
  using EventHandler_2 = EventHandler_t<PeerConnect*, string>;
  using EventHandler_3 = EventHandler_t<PeerConnect*, string, Data&>;
  using Events = std::map<string, std::unique_ptr<Handler_t>>;
  using MessageHandler = std::function<void( PeerConnect*, string, Buffer& )>;

  //
  // ControlObserver implementation
  //

  void OnSignedIn( const string id );
  void OnSignedOut( const string id );
  void OnPeerConnected( const string id );
  void OnPeerDisconnected( const string id );
  void OnPeerMessage( const string id, const char* buffer, const size_t size );
  void OnPeerWritable( const string id );
  void OnError( const string id, const string& reason );


  bool ParseOptions( const string& options );
  std::string tolower( const string& str );

  bool signout_;
  Setting setting_;
  Events event_handler_;
  MessageHandler message_handler_;

  std::shared_ptr<Control> control_;
  std::shared_ptr<Signal> signal_;

  string error_reason_;
};


} // namespace pc

#endif // __PEERCONNECT_PEERCONNECT_H__
