/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include "control.h"
#include "peer.h"
#include "webrtc/api/test/fakedtlsidentitystore.h"
#include "webrtc/p2p/client/fakeportallocator.h"
#include "webrtc/api/test/mockpeerconnectionobservers.h"


namespace tn {

//
// class PeerControl
//

PeerControl::PeerControl(const std::string local_session_id,
                         const std::string remote_session_id,
                         PeerObserver* observer,
                         rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
                             peer_connection_factory)
    : local_session_id_(local_session_id),
      remote_session_id_(remote_session_id),
      observer_(observer),
      peer_connection_factory_(peer_connection_factory){

  if (!CreatePeerConnection(NULL)) {
    LOG(LS_ERROR) << "CreatePeerConnection failed";
    DeletePeerConnection();
  }

  webrtc::DataChannelInit init;
  const std::string data_channel_name = std::string("tn_data_") + remote_session_id_;
  if (!CreateDataChannel(data_channel_name, init)) {
    LOG(LS_ERROR) << "CreateDataChannel failed";
    DeletePeerConnection();
  }
}

PeerControl::~PeerControl() {
  DeletePeerConnection();
  LOG(LS_INFO) << "PeerControl has been deleted";
}


bool PeerControl::Send(const std::string& message) {
  return local_data_channel_->Send(message);
}


void PeerControl::CreateOffer(const webrtc::MediaConstraintsInterface* constraints) {
  peer_connection_->CreateOffer(this, constraints);
}


void PeerControl::CreateAnswer(
  const webrtc::MediaConstraintsInterface* constraints) {
  peer_connection_->CreateAnswer(this, constraints);
}


void PeerControl::ReceiveOfferSdp(const std::string& sdp) {
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kOffer, sdp);
  CreateAnswer(NULL);
}


void PeerControl::ReceiveAnswerSdp(const std::string& sdp) {
  SetRemoteDescription(webrtc::SessionDescriptionInterface::kAnswer, sdp);
}


void PeerControl::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  PeerDataChannelObserver* Observer = new PeerDataChannelObserver(data_channel);
  remote_data_channel_ = rtc::scoped_ptr<PeerDataChannelObserver>(Observer);
  SigslotConnect(remote_data_channel_.get());
}

void PeerControl::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string sdp;
  if (!candidate->ToString(&sdp)) return;

  Json::Value data;

  data["sdp_mid"] = candidate->sdp_mid();
  data["sdp_mline_index"] = candidate->sdp_mline_index();
  data["candidate"] = sdp;

  observer_->SendCommand("ice_candidate", data, remote_session_id_);
}

void PeerControl::OnSuccess(webrtc::SessionDescriptionInterface* desc) {

  // This callback should take the ownership of |desc|.
  rtc::scoped_ptr<webrtc::SessionDescriptionInterface> owned_desc(desc);
  std::string sdp;

  if (!desc->ToString(&sdp)) return;

  // Set local description
  SetLocalDescription(desc->type(), sdp);

  //
  // Send message to other peer
  Json::Value data;

  if (desc->type() == webrtc::SessionDescriptionInterface::kOffer) {
    data["sdp"] = sdp;
    observer_->SendCommand("offersdp", data, remote_session_id_);
  }
  else if (desc->type() == webrtc::SessionDescriptionInterface::kAnswer) {
    data["sdp"] = sdp;
    observer_->SendCommand("answersdp", data, remote_session_id_);
  }
}

void PeerControl::OnPeerOpened() {
  if (local_data_channel_.get() != nullptr && remote_data_channel_.get() != nullptr &&
    local_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen &&
    remote_data_channel_->state() == webrtc::DataChannelInterface::DataState::kOpen
    ) {
    observer_->OnConnected(remote_session_id_);
  }
}


void PeerControl::OnPeerMessage(const webrtc::DataBuffer& buffer) {
  std::string data;
  observer_->OnData(remote_session_id_, buffer.data.data<char>(), buffer.data.size());
}


bool PeerControl::CreateDataChannel(
                    const std::string& label,
                    const webrtc::DataChannelInit& init) {

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;

  data_channel = peer_connection_->CreateDataChannel(label, &init);
  if (data_channel.get() == NULL) {
    return false;
  }

  local_data_channel_.reset(new PeerDataChannelObserver(data_channel));
  if (local_data_channel_.get() == NULL) {
    return false;
  }

  SigslotConnect(local_data_channel_.get());
  return true;
}

void PeerControl::AddIceCandidate(const std::string& sdp_mid, int sdp_mline_index,
                                  const std::string& candidate) {

  rtc::scoped_ptr<webrtc::IceCandidateInterface> owned_candidate(
    webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, NULL));

  peer_connection_->AddIceCandidate(owned_candidate.get());
}


bool PeerControl::CreatePeerConnection(
        const webrtc::MediaConstraintsInterface* constraints) {
  ASSERT(peer_connection_factory_.get() != NULL);
  ASSERT(peer_connection_.get() == NULL);

  rtc::scoped_ptr<cricket::PortAllocator> port_allocator(
    new cricket::FakePortAllocator(rtc::Thread::Current(), nullptr));

  // CreatePeerConnection with RTCConfiguration.
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  webrtc::PeerConnectionInterface::IceServer ice_server;
  ice_server.uri = "stun:stun.l.google.com:19302";
  config.servers.push_back(ice_server);

  rtc::scoped_ptr<webrtc::DtlsIdentityStoreInterface> dtls_identity_store(
    rtc::SSLStreamAdapter::HaveDtlsSrtp() ?
    new FakeDtlsIdentityStore() : nullptr);

  peer_connection_ = peer_connection_factory_->CreatePeerConnection(
    config, constraints, std::move(port_allocator),
    std::move(dtls_identity_store), this);

  return peer_connection_.get() != NULL;
}

void PeerControl::DeletePeerConnection() {
  remote_data_channel_ = NULL;
  local_data_channel_ = NULL;
  peer_connection_ = NULL;
  peer_connection_factory_ = NULL;
}

void PeerControl::SetLocalDescription(const std::string& type,
                                              const std::string& sdp) {

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetLocalDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

void PeerControl::SetRemoteDescription(const std::string& type,
                                               const std::string& sdp) {

  rtc::scoped_refptr<webrtc::MockSetSessionDescriptionObserver>
    observer(new rtc::RefCountedObject<
      webrtc::MockSetSessionDescriptionObserver>());
  peer_connection_->SetRemoteDescription(
    observer, webrtc::CreateSessionDescription(type, sdp, NULL));
}

void PeerControl::SigslotConnect(PeerDataChannelObserver* datachanel) {
  datachanel->SignalOnOpen_.connect(this, &PeerControl::OnPeerOpened);
  datachanel->SignalOnMessage_.connect(this, &PeerControl::OnPeerMessage);
}




//
// class PeerDataChannelObserver
//

PeerDataChannelObserver::PeerDataChannelObserver(webrtc::DataChannelInterface* channel)
  : channel_(channel), received_message_count_(0) {
  channel_->RegisterObserver(this);
  state_ = channel_->state();
}

PeerDataChannelObserver::~PeerDataChannelObserver() {
  channel_->Close();
  channel_->UnregisterObserver();
}

void PeerDataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount) {
  return;
}

void PeerDataChannelObserver::OnStateChange() {
  state_ = channel_->state();
  if (state_ == webrtc::DataChannelInterface::DataState::kOpen) {
    SignalOnOpen_();
  }
}

void PeerDataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  SignalOnMessage_(buffer);
  ++received_message_count_;
}

bool PeerDataChannelObserver::Send(const std::string& message) {
  webrtc::DataBuffer buffer(message);
  return channel_->Send(buffer);
}

void PeerDataChannelObserver::Close() {
  channel_->Close();
}


bool PeerDataChannelObserver::IsOpen() const {
  return state_ == webrtc::DataChannelInterface::kOpen;
}

const webrtc::DataChannelInterface::DataState
PeerDataChannelObserver::state() const {
  return channel_->state();
}

size_t PeerDataChannelObserver::received_message_count() const {
  return received_message_count_;
}

} // namespace tn