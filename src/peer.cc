/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include "control.h"
#include "peer.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/api/test/mockpeerconnectionobservers.h"

#include "logging.h"

namespace pc {

//
// class PeerControl
//

PeerControl::PeerControl(const std::string local_id,
                         const std::string remote_id,
                         PeerObserver* observer,
                         rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
                             peer_connection_factory)
    : local_id_(local_id),
      remote_id_(remote_id),
      control_(observer),
      peer_connection_factory_(peer_connection_factory),
      state_(pClosed) {

}

PeerControl::~PeerControl() {
  ASSERT(state_ == pClosed);
  DeletePeerConnection();
  LOGP_F( INFO ) << "Done";
}


bool PeerControl::Initialize() {

  if (!CreatePeerConnection()) {
    LOGP_F(LS_ERROR) << "CreatePeerConnection failed";
    DeletePeerConnection();
    return false;
  }

  webrtc::DataChannelInit init;
  const std::string data_channel_name = std::string("pc_data_") + remote_id_;
  if (!CreateDataChannel(data_channel_name, init)) {
    LOGP_F(LS_ERROR) << "CreateDataChannel failed";
    DeletePeerConnection();
    return false;
  }

  LOGP_F( INFO ) << "Done";
  return true;
}

bool PeerControl::Send(const char* buffer, const size_t size) {
  ASSERT( state_ == pOpen );
  
  if ( state_ != pOpen ) {
    LOGP_F( WARNING ) << "Send data when a peer state is not opened";
    return false;
  }

  return local_data_channel_->Send(buffer, size);
}

bool PeerControl::SyncSend(const char* buffer, const size_t size) {
  ASSERT( state_ == pOpen );

  if ( state_ != pOpen ) {
    LOGP_F( WARNING ) << "Send data when a peer state is not opened";
    return false;
  }

  return local_data_channel_->SyncSend(buffer, size);
}

bool PeerControl::IsWritable() {

  if ( state_ != pOpen ) {
    LOGP_F( WARNING ) << "A function was called when a peer state is not opened";
    return false;
  }

  return local_data_channel_->IsWritable();
}

void PeerControl::Close() {
  LOGP_F_IF(state_ != pOpen, WARNING) << "Closing peer when it is not opened";

  if ( state_ == pClosing || state_ == pClosed ) {
    LOGP_F( WARNING ) << "Close peer when is closing or already closed";
    return;
  }

  state_ = pClosing;

  LOGP_F( INFO ) << "Close data-channel of remote_id_ " << remote_id_;

  if ( peer_connection_ ) {
    peer_connection_->Close();
  }
  else {
    LOGP_F( WARNING ) << "peer_connection_ is nullptr ";
    state_ = pClosed;
  }
}


void PeerControl::CreateOffer(const webrtc::MediaConstraintsInterface* constraints) {
  ASSERT( state_ == pClosed );

  state_ = pConnecting;
  peer_connection_->CreateOffer(this, constraints);
  LOGP_F( INFO ) << "Done";
}


void PeerControl::CreateAnswer(const webrtc::MediaConstraintsInterface* constraints) {
  ASSERT( state_ == pClosed );

  state_ = pConnecting;
  peer_connection_->CreateAnswer(this, constraints);
  LOGP_F( INFO ) << "Done";
}


void PeerControl::ReceiveOfferSdp(const std::string& sdp) {
  ASSERT( state_ == pClosed);
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kOffer, sdp);
  CreateAnswer(NULL);
  LOGP_F( INFO ) << "Done";
}


void PeerControl::ReceiveAnswerSdp(const std::string& sdp) {
  ASSERT( state_ == pConnecting );
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kAnswer, sdp);
  LOGP_F( INFO ) << "Done";
}

void PeerControl::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  LOGP_F( INFO ) << "remote_id_ is " << remote_id_;

  PeerDataChannelObserver* Observer = new PeerDataChannelObserver(data_channel);
  remote_data_channel_ = std::unique_ptr<PeerDataChannelObserver>(Observer);
  Attach(remote_data_channel_.get());

  LOGP_F( INFO ) << "Done";
}

void PeerControl::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  // NOTHING
}

void PeerControl::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  switch (new_state) {
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:
    //
    // Ice connection has been closed.
    // Notify it to Control so the Control will remove peer in peers_
    //
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionClosed";
    OnPeerClosed();
    break;

  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:
    //
    // Peer disconnected and notify it to control that makes control trigger closing
    //
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionDisconnected";
    OnPeerDisconnected();
    break;
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionNew";
    break;
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionChecking";
    break;
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionConnected";
    break;
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionCompleted";
    break;
  case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:
    LOGP_F( INFO ) << "new_state is " << "kIceConnectionFailed";
    break;
  default:
    break;
  }
}


void PeerControl::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string sdp;
  if (!candidate->ToString(&sdp)) return;

  Json::Value data;

  data["sdp_mid"] = candidate->sdp_mid();
  data["sdp_mline_index"] = candidate->sdp_mline_index();
  data["candidate"] = sdp;

  control_->SendCommand(remote_id_, "ice_candidate", data);
  LOGP_F( INFO ) << "Done";
}

void PeerControl::OnSuccess(webrtc::SessionDescriptionInterface* desc) {

  // This callback should take the ownership of |desc|.
  std::unique_ptr<webrtc::SessionDescriptionInterface> owned_desc(desc);
  std::string sdp;

  if (!desc->ToString(&sdp)) return;

  // Set local description
  SetLocalDescription(desc->type(), sdp);

  //
  // Send message to other peer
  Json::Value data;

  if (desc->type() == webrtc::SessionDescriptionInterface::kOffer) {
    data["sdp"] = sdp;
    control_->SendCommand(remote_id_, "offersdp", data);
  }
  else if (desc->type() == webrtc::SessionDescriptionInterface::kAnswer) {
    data["sdp"] = sdp;
    control_->SendCommand(remote_id_, "answersdp", data);
  }
  LOGP_F( INFO ) << "Done";
}

void PeerControl::OnPeerOpened() {

  // Both local_data_channel_ and remote_data_channel_ has been opened
  if (local_data_channel_.get() != nullptr && remote_data_channel_.get() != nullptr &&
      local_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen &&
      remote_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen
    ) {
    LOG_F( INFO ) << "Peers are connected, " << remote_id_ << " and " << local_id_;
    ASSERT( state_ == pConnecting );
 
    // Fianlly, data-channel has been opened.
    state_ = pOpen;
    control_->OnConnected(remote_id_);
    control_->OnWritable(local_id_);
  }

  LOGP_F( INFO ) << "Done";
}

void PeerControl::OnPeerClosed() {

  ASSERT( state_ == pClosing );

  state_ = pClosed;
  control_->OnClosed(remote_id_);

}

void PeerControl::OnPeerDisconnected() {

  if ( state_ == pClosed ) {
    LOGP_F( WARNING ) << "Already closed";
    return;
  }
  else if ( state_ == pClosing ) {
    LOGP_F( INFO ) << "Already closing";
    return;
  }

  control_->Close( remote_id_ );
}


void PeerControl::OnPeerMessage(const webrtc::DataBuffer& buffer) {
  std::string data;
  control_->OnMessage(remote_id_, buffer.data.data<char>(), buffer.data.size());
}

void PeerControl::OnBufferedAmountChange(const uint64_t previous_amount) {
  if ( !local_data_channel_->IsWritable() ) {
    LOGP_F( LERROR ) << "local_data_channel_ is not writable";
    return;
  }
  control_->OnWritable( remote_id_ );
}


bool PeerControl::CreateDataChannel(
                    const std::string& label,
                    const webrtc::DataChannelInit& init) {

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;

  data_channel = peer_connection_->CreateDataChannel(label, &init);
  if (data_channel.get() == nullptr) {
    LOGP_F( LERROR ) << "data_channel is null";
    return false;
  }

  local_data_channel_.reset(new PeerDataChannelObserver(data_channel));
  if (local_data_channel_.get() == NULL) {
    LOGP_F( LERROR ) << "local_data_channel_ is null";
    return false;
  }

  Attach(local_data_channel_.get());

  LOGP_F( INFO ) << "Done";
  return true;
}

void PeerControl::AddIceCandidate(const std::string& sdp_mid, int sdp_mline_index,
                                  const std::string& candidate) {

  std::unique_ptr<webrtc::IceCandidateInterface> owned_candidate(
    webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, NULL));

  peer_connection_->AddIceCandidate(owned_candidate.get());
  LOGP_F( INFO ) << "Done";
}


bool PeerControl::CreatePeerConnection() {
  ASSERT(peer_connection_factory_.get() != NULL);
  ASSERT(peer_connection_.get() == NULL);

  // Enable DTLS
  webrtc::FakeConstraints constraints;
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true");

  // CreatePeerConnection with RTCConfiguration.
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  webrtc::PeerConnectionInterface::IceServer ice_server;
  ice_server.uri = "stun:stun.l.google.com:19302";
  config.servers.push_back(ice_server);

  peer_connection_ = peer_connection_factory_->CreatePeerConnection(
    config, &constraints, NULL, NULL, this);

  if ( peer_connection_.get() == nullptr ) {
    LOGP_F( LERROR ) << "peer_connection is null";
    return false;
  }

  return true;
}

void PeerControl::DeletePeerConnection() {
  Detach(remote_data_channel_.get());
  Detach(local_data_channel_.get());

  remote_data_channel_ = NULL;
  local_data_channel_ = NULL;
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;

  LOGP_F( INFO ) << "Done";
}

void PeerControl::SetLocalDescription(const std::string& type,
                                              const std::string& sdp) {

  if ( peer_connection_ == nullptr ) {
    LOGP_F( LERROR ) << "peer_connection_ is nullptr";
    return;
  }

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetLocalDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));

  LOGP_F( INFO ) << "Done";
}

void PeerControl::SetRemoteDescription(const std::string& type,
                                               const std::string& sdp) {

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetRemoteDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));

  LOGP_F( INFO ) << "Done";
}

void PeerControl::Attach(PeerDataChannelObserver* datachannel) {
  if (datachannel == nullptr) {
    LOGP_F(WARNING) << "Attach to nullptr";
    return;
  }

  datachannel->SignalOnOpen_.connect(this, &PeerControl::OnPeerOpened);
  datachannel->SignalOnDisconnected_.connect(this, &PeerControl::OnPeerDisconnected);
  datachannel->SignalOnMessage_.connect(this, &PeerControl::OnPeerMessage);
  datachannel->SignalOnBufferedAmountChange_.connect(this, &PeerControl::OnBufferedAmountChange);
  LOGP_F( INFO ) << "Done";
}

void PeerControl::Detach(PeerDataChannelObserver* datachannel) {
  if (datachannel == nullptr) {
    LOGP_F(WARNING) << "Detach from nullptr";
    return;
  }

  datachannel->SignalOnOpen_.disconnect(this);
  datachannel->SignalOnDisconnected_.disconnect(this);
  datachannel->SignalOnMessage_.disconnect(this);
  datachannel->SignalOnBufferedAmountChange_.disconnect(this);
  LOGP_F( INFO ) << "Done";
}



//
// class PeerDataChannelObserver
//

PeerDataChannelObserver::PeerDataChannelObserver(webrtc::DataChannelInterface* channel)
  : channel_(channel) {
  channel_->RegisterObserver(this);
  state_ = channel_->state();
  LOGP_F( INFO ) << "Done";
}

PeerDataChannelObserver::~PeerDataChannelObserver() {
  channel_->Close();
  state_ = channel_->state();
  channel_->UnregisterObserver();
  LOGP_F( INFO ) << "Done";
}

void PeerDataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount) {
  SignalOnBufferedAmountChange_(previous_amount);

  if (channel_->buffered_amount() == 0) {
    std::lock_guard<std::mutex> lk(send_lock_);
    send_cv_.notify_all();
  }

  return;
}

void PeerDataChannelObserver::OnStateChange() {
  state_ = channel_->state();
  if (state_ == webrtc::DataChannelInterface::DataState::kOpen) {
    LOGP_F( INFO ) << "Data channel internal state is kOpen";
    SignalOnOpen_();
  }
  else if (state_ == webrtc::DataChannelInterface::DataState::kClosed) {
    LOGP_F( INFO ) << "Data channel internal state is kClosed";
    SignalOnDisconnected_();
  }
}

void PeerDataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  SignalOnMessage_(buffer);
}

bool PeerDataChannelObserver::Send(const char* buffer, const size_t size) {
  rtc::CopyOnWriteBuffer rtcbuffer(buffer, size);
  webrtc::DataBuffer databuffer(rtcbuffer, true);

  if ( channel_->buffered_amount() >= max_buffer_size_ ) {
    LOGP_F( LERROR ) << "Buffer is full";
    return false;
  }

  return channel_->Send(databuffer);
}

bool PeerDataChannelObserver::SyncSend(const char* buffer, const size_t size) {
  rtc::CopyOnWriteBuffer rtcbuffer(buffer, size);
  webrtc::DataBuffer databuffer(rtcbuffer, true);

  std::unique_lock<std::mutex> lock(send_lock_);
  if (!channel_->Send(databuffer)) return false;

  if (!send_cv_.wait_for(lock, std::chrono::milliseconds(60*1000),
                         [this] () { return channel_->buffered_amount() == 0; })) {
    LOGP_F( LERROR ) << "Buffer is full";
    return false;
  }

  return true;
}

void PeerDataChannelObserver::Close() {
  LOGP_F(LS_INFO) << "Closing data channel";
  if (channel_->state() != webrtc::DataChannelInterface::kClosing) {
    channel_->Close();
  }
}


bool PeerDataChannelObserver::IsOpen() const {
  return state_ == webrtc::DataChannelInterface::kOpen;
}

uint64_t PeerDataChannelObserver::BufferedAmount() {
  return channel_->buffered_amount();
}

bool PeerDataChannelObserver::IsWritable() {
  if ( channel_ == nullptr ) {
    LOGP_F( LERROR ) << "channel_ is null";
    return false;
  }

  if ( !IsOpen() ) {
    LOGP_F( LERROR ) << "data channel is not opened";
    return false;
  }

  if ( channel_->buffered_amount() > 0 ) {
    LOGP_F( LERROR ) << "buffer is full";
    return false;
  }

  return true;
}


const webrtc::DataChannelInterface::DataState
PeerDataChannelObserver::state() const {
  return channel_->state();
}


} // namespace pc