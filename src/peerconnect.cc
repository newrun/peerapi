/*
 *  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
 *
 *  Ryan Lee
 */

#include <string>
#include <locale>
 
#include "peerconnect.h"
#include "control.h"
#include "logging.h"

namespace pc {

PeerConnect::PeerConnect( const string channel ) {
  // Log level
#if DEBUG || _DEBUG
  rtc::LogMessage::LogToDebug( rtc::LS_NONE );
  pc::LogMessage::LogToDebug( pc::WARNING );
#else
  rtc::LogMessage::LogToDebug( rtc::LS_NONE );
  pc::LogMessage::LogToDebug( pc::LS_NONE );
#endif

  string local_channel;

  if ( channel.empty() ) {
    local_channel = rtc::CreateRandomUuid();
  }
  else {
    local_channel = channel;
  }

  channel_ = local_channel;
  close_once_ = false;

  LOGP_F( INFO ) << "Done";
}

PeerConnect::~PeerConnect() {
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Run() {
  rtc::ThreadManager::Instance()->CurrentThread()->Run();
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Stop() {
  rtc::ThreadManager::Instance()->CurrentThread()->Quit();
  LOGP_F( INFO ) << "Done";
}


void PeerConnect::Open() {

  if ( control_.get() != nullptr ) {
    LOGP_F( WARNING ) << "Already open.";
    return;
  }

  //
  // create signal client
  //

  if ( signal_ == nullptr ) {
    signal_ = std::make_shared<pc::Signal>( setting_.signal_uri_ );
  }

  //
  // Initialize control
  //

  control_ = std::make_shared<pc::Control>( signal_ );
  control_->RegisterObserver( this, control_ );

  if ( control_.get() == NULL ) {
    LOGP_F( LERROR ) << "Failed to create class Control.";
    return;
  }

  //
  // Initialize peer connection
  //

  if ( !control_->InitializeControl() ) {
    LOGP_F( LERROR ) << "Failed to initialize Control.";
    control_.reset();
    return;
  }

  //
  // Connect to signal server
  //

  control_->Open( setting_.signal_id_, setting_.signal_password_, channel_ );
  LOGP_F( INFO ) << "Done";
  return;
}

void PeerConnect::Close( const string channel ) {

  if ( channel.empty() || channel == channel_ ) {
    control_->Close( CLOSE_NORMAL, FORCE_QUEUING_ON );
    signal_->SyncClose();
  }
  else {
    control_->ClosePeer( channel, CLOSE_NORMAL, FORCE_QUEUING_ON );
  }
  LOGP_F( INFO ) << "Done";
}

void PeerConnect::Connect( const string channel ) {
  control_->Connect( channel );
  LOGP_F( INFO ) << "Done, channel is " << channel;
  return;
}

//
// Send message to destination peer session id
//

bool PeerConnect::Send( const string& channel, const char* buffer, const size_t size, const bool wait ) {
  if ( wait ) {

    //
    // Synchronous send returns true or false
    // and a timeout is 60*1000 ms by default.
    //

    return control_->SyncSend( channel, buffer, size );
  }
  else {
    control_->Send( channel, buffer, size );

    //
    // Asyncronous send always returns true and
    // trigger 'close' event with CloseCode if failed
    //

    return true;
  }
}

bool PeerConnect::Send( const string& channel, const char* message, const bool wait  ) {
  return Send( channel, message, strlen( message ), wait );
}

bool PeerConnect::Send( const string& channel, const string& message, const bool wait  ) {
  return Send( channel, message.c_str(), message.size(), wait );
}

bool PeerConnect::SetOptions( const string options ) {

  // parse settings
  if ( !options.empty() ) {
    return ParseOptions( options );
  }
  return true;
}


std::string PeerConnect::CreateRandomUuid() {
  return rtc::CreateRandomUuid();
}

//
// Register Event handler
//
PeerConnect& PeerConnect::On( string event_id, std::function<void( string )> handler ) {

  if ( event_id.empty() ) return *this;

  if ( event_id == "open" || event_id == "connect" || event_id == "writable" ) {
    std::unique_ptr<EventHandler_2> f( new EventHandler_2( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );
    LOGP_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOGP_F( LERROR ) << "Unsupported event type: " << event_id;
  }
  return *this;
}

PeerConnect& PeerConnect::On( string event_id, std::function<void( string, string )> handler ) {

  if ( event_id.empty() ) return *this;

  LOGP_F( LERROR ) << "Unsupported event type: " << event_id;
  return *this;
}

PeerConnect& PeerConnect::On( string event_id, std::function<void( string, pc::CloseCode, string )> handler ) {
  if ( event_id.empty() ) return *this;

  if ( event_id == "close" ) {
    std::unique_ptr<EventHandler_Close> f( new EventHandler_Close( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );

    LOGP_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOGP_F( LERROR ) << "Unsupported event type: " << event_id;
  }

  return *this;
}

PeerConnect& PeerConnect::On( string event_id, std::function<void( string, Buffer& )> handler ) {
  if ( event_id.empty() ) return *this;

  if ( event_id == "message" ) {
    std::unique_ptr<EventHandler_Message> f( new EventHandler_Message( handler ) );
    event_handler_.insert( Events::value_type( event_id, std::move( f ) ) );

    LOGP_F( INFO ) << "An event handler '" << event_id << "' has been inserted";
  }
  else {
    LOGP_F( LERROR ) << "Unsupported event type: " << event_id;
  }

  return *this;
}

//
// Signal event handler
//

void PeerConnect::OnOpen( const string channel ) {
  close_once_ = false;

  if ( event_handler_.find( "open" ) != event_handler_.end() ) {
    CallEventHandler( "open", channel );
  }

  LOGP_F( INFO ) << "Done";
}

void PeerConnect::OnClose( const string channel, const CloseCode code, const string desc ) {

  // This instance of PeerConnect and local channel is going to be closed
  if ( channel == channel_ ) {
    if ( close_once_ ) {
      LOGP_F( WARNING ) << "close_ is false, channel is " << channel;
      return;
    }

    close_once_ = true;

    if ( event_handler_.find( "close" ) != event_handler_.end() ) {
      CallEventHandler( "close", channel, code, desc );
    }

    control_->UnregisterObserver();
    control_.reset();
  }
  // Remote peer has been closed
  else {
    if ( event_handler_.find( "close" ) != event_handler_.end() ) {
      CallEventHandler( "close", channel, code, desc );
    }
  }

  LOGP_F( INFO ) << "Done, channel is " << channel;
}

void PeerConnect::OnConnect( const string channel ) {
  if ( event_handler_.find( "connect" ) != event_handler_.end() ) {
    CallEventHandler( "connect", channel );
  }

  LOGP_F( INFO ) << "Done, channel is " << channel;
}

void PeerConnect::OnMessage( const string channel, const char* buffer, const size_t size ) {
  Buffer buf( buffer, size );

  if ( event_handler_.find( "message" ) != event_handler_.end() ) {
    CallEventHandler( "message", channel, buf );
  }
}

void PeerConnect::OnWritable( const string channel ) {
  if ( event_handler_.find( "writable" ) != event_handler_.end() ) {
    CallEventHandler( "writable", channel );
  }

  LOGP_F( INFO ) << "Done, channel is " << channel;
}


template<typename ...A>
void PeerConnect::CallEventHandler( string msg_id, A&& ... args )
{
  using eventhandler_t = EventHandler_t<A...>;
  using cb_t = std::function<void( A... )>;
  const Handler_t& base = *event_handler_[msg_id];
  const cb_t& func = static_cast<const eventhandler_t&>( base ).callback_;
  func( std::forward<A>( args )... );
}


bool PeerConnect::ParseOptions( const string& options ) {
  Json::Reader reader;
  Json::Value joptions;

  string value;

  if ( !reader.parse( options, joptions ) ) {
    LOGP_F( WARNING ) << "Invalid setting: " << options;
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

} // namespace pc