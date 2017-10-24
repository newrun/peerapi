/*
 *  Copyright 2016 The PeerApi Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#include <string>
#include <locale>
 
#include "peerconnect.h"
#include "control.h"
#include "logging.h"

namespace peerapi {

Peer::Peer( const string peer_id ) {
  // Log level
#if DEBUG || _DEBUG
  rtc::LogMessage::LogToDebug( rtc::LS_NONE );
  peerapi::LogMessage::LogToDebug( peerapi::WARNING );
#else
  rtc::LogMessage::LogToDebug( rtc::LS_NONE );
  peerapi::LogMessage::LogToDebug( peerapi::LS_NONE );
#endif

  string local_peer_id;

  if ( peer_id.empty() ) {
    local_peer_id = rtc::CreateRandomUuid();
  }
  else {
    local_peer_id = peer_id;
  }

  peer_id_ = local_peer_id;
  close_once_ = false;

  LOG_F( INFO ) << "Done";
}

Peer::~Peer() {
  LOG_F( INFO ) << "Done";
}

void Peer::Run() {
  rtc::ThreadManager::Instance()->CurrentThread()->Run();
  LOG_F( INFO ) << "Done";
}

void Peer::Stop() {
  rtc::ThreadManager::Instance()->CurrentThread()->Quit();
  LOG_F( INFO ) << "Done";
}


void Peer::Open() {

  if ( control_.get() != nullptr ) {
    LOG_F( WARNING ) << "Already open.";
    return;
  }

  //
  // create signal client
  //

  if ( signal_ == nullptr ) {
    signal_ = std::make_shared<peerapi::Signal>( setting_.signal_uri_ );
  }

  //
  // Initialize control
  //

  control_ = std::make_shared<peerapi::Control>( signal_ );
  control_->RegisterObserver( this, control_ );

  if ( control_.get() == NULL ) {
    LOG_F( LERROR ) << "Failed to create class Control.";
    return;
  }

  //
  // Initialize peer connection
  //

  if ( !control_->InitializeControl() ) {
    LOG_F( LERROR ) << "Failed to initialize Control.";
    control_.reset();
    return;
  }

  //
  // Connect to signal server
  //

  control_->Open( setting_.signal_id_, setting_.signal_password_, peer_id_ );
  LOG_F( INFO ) << "Done";
  return;
}

void Peer::Close( const string peer_id ) {

  if ( peer_id.empty() || peer_id == peer_id_ ) {
    control_->Close( CLOSE_NORMAL, FORCE_QUEUING_ON );
    signal_->SyncClose();
  }
  else {
    control_->ClosePeer( peer_id, CLOSE_NORMAL, FORCE_QUEUING_ON );
  }
  LOG_F( INFO ) << "Done";
}

void Peer::Connect( const string peer_id ) {
  control_->Connect( peer_id );
  LOG_F( INFO ) << "Done, peer is " << peer_id;
  return;
}

//
// Send message to destination peer session id
//

bool Peer::Send( const string& peer_id, const char* data, const size_t size, const bool wait ) {
  if ( wait ) {

    //
    // Synchronous send returns true or false
    // and a timeout is 60*1000 ms by default.
    //

    return control_->SyncSend( peer_id, data, size );
  }
  else {
    control_->Send( peer_id, data, size );

    //
    // Asyncronous send always returns true and
    // trigger 'close' event with CloseCode if failed
    //

    return true;
  }
}

bool Peer::Send( const string& peer_id, const string& message, const bool wait  ) {
  return Send( peer_id, message.c_str(), message.size(), wait );
}

bool Peer::SetOptions( const string options ) {

  // parse settings
  if ( !options.empty() ) {
    return ParseOptions( options );
  }
  return true;
}


std::string Peer::CreateRandomUuid() {
  return rtc::CreateRandomUuid();
}

//
// Register Event handler
//
Peer& Peer::On( string event_id, std::function<void( string )> handler ) {

  if ( event_id.empty() ) return *this;

  if ( event_id == "open" || event_id == "connect" || event_id == "writable" ) {
    std::unique_ptr<EventHandler_2> f( new EventHandler_2( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );
    LOG_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOG_F( LERROR ) << "Unsupported event type: " << event_id;
  }
  return *this;
}

Peer& Peer::On( string event_id, std::function<void( string, string )> handler ) {

  if ( event_id.empty() ) return *this;

  LOG_F( LERROR ) << "Unsupported event type: " << event_id;
  return *this;
}

Peer& Peer::On( string event_id, std::function<void( string, peerapi::CloseCode, string )> handler ) {
  if ( event_id.empty() ) return *this;

  if ( event_id == "close" ) {
    std::unique_ptr<EventHandler_Close> f( new EventHandler_Close( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );

    LOG_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOG_F( LERROR ) << "Unsupported event type: " << event_id;
  }

  return *this;
}

Peer& Peer::On( string event_id, std::function<void( string, char*, std::size_t )> handler ) {
  if ( event_id.empty() ) return *this;

  if ( event_id == "message" ) {
    std::unique_ptr<EventHandler_Message> f( new EventHandler_Message( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );

    LOG_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOG_F( LERROR ) << "Unsupported event type: " << event_id;
  }

  return *this;
}

//
// Signal event handler
//

void Peer::OnOpen( const string peer_id ) {
  close_once_ = false;

  if ( event_handler_.find( "open" ) != event_handler_.end() ) {
    CallEventHandler( "open", peer_id );
  }

  LOG_F( INFO ) << "Done";
}

void Peer::OnClose( const string peer_id, const CloseCode code, const string desc ) {

  // This instance of Peer and local peer is going to be closed
  if ( peer_id == peer_id_ ) {
    if ( close_once_ ) {
      LOG_F( WARNING ) << "close_ is false, peer is " << peer_id;
      return;
    }

    close_once_ = true;

    if ( event_handler_.find( "close" ) != event_handler_.end() ) {
      CallEventHandler( "close", peer_id, code, desc );
    }

    control_->UnregisterObserver();
    control_.reset();
  }
  // Remote peer has been closed
  else {
    if ( event_handler_.find( "close" ) != event_handler_.end() ) {
      CallEventHandler( "close", peer_id, code, desc );
    }
  }

  LOG_F( INFO ) << "Done, peer is " << peer_id;
}

void Peer::OnConnect( const string peer_id ) {
  if ( event_handler_.find( "connect" ) != event_handler_.end() ) {
    CallEventHandler( "connect", peer_id );
  }

  LOG_F( INFO ) << "Done, peer is " << peer_id;
}

void Peer::OnMessage( const string peer_id, const char* data, const size_t size ) {
  if ( event_handler_.find( "message" ) != event_handler_.end() ) {
    CallEventHandler( "message", peer_id, data, size );
  }
}

void Peer::OnWritable( const string peer_id ) {
  if ( event_handler_.find( "writable" ) != event_handler_.end() ) {
    CallEventHandler( "writable", peer_id );
  }

  LOG_F( INFO ) << "Done, peer is " << peer_id;
}


template<typename ...A>
void Peer::CallEventHandler( string msg_id, A&& ... args )
{
  using eventhandler_t = EventHandler_t<A...>;
  using cb_t = std::function<void( A... )>;
  const Handler_t& base = *event_handler_[msg_id];
  const cb_t& func = static_cast<const eventhandler_t&>( base ).callback_;
  func( std::forward<A>( args )... );
}


bool Peer::ParseOptions( const string& options ) {
  Json::Reader reader;
  Json::Value joptions;

  string value;

  if ( !reader.parse( options, joptions ) ) {
    LOG_F( WARNING ) << "Invalid setting: " << options;
    return false;
  }

  if ( rtc::GetStringFromJsonObject( joptions, "url", &value ) ) {
    setting_.signal_uri_ = value;
  }

  if ( rtc::GetStringFromJsonObject( joptions, "user_id", &value ) ) {
    setting_.signal_id_ = value;
  }

  if ( rtc::GetStringFromJsonObject( joptions, "user_password", &value ) ) {
    setting_.signal_password_ = value;
  }

  return true;
}

} // namespace peerapi