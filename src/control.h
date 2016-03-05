#ifndef __THROUGHNET_CONTROL_H__
#define __THROUGHNET_CONTROL_H__
#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "dummy_wnd.h"
#include "connection.h"
#include "webrtc/base/scoped_ptr.h"

namespace webrtc {
class VideoCaptureModule;
}  // namespace webrtc

namespace cricket {
class VideoRenderer;
}  // namespace cricket

class Control
  : public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver,
    public ConnectionObserver,
    public MainWndCallback {
 public:
  enum CallbackID {
    MEDIA_CHANNELS_INITIALIZED = 1,
    PEER_CONNECTION_CLOSED,
    SEND_MESSAGE_TO_PEER,
    NEW_STREAM_ADDED,
    STREAM_REMOVED,
  };

  Control(Connection* client, MainWindow* main_wnd);

  bool connection_active() const;

  virtual void Close();

 protected:
  ~Control();
  bool InitializePeerConnection();
  bool ReinitializePeerConnectionForLoopback();
  bool CreatePeerConnection(bool dtls);
  void DeletePeerConnection();
  void EnsureStreamingUI();
  void AddStreams();
  cricket::VideoCapturer* OpenVideoCaptureDevice();

  //
  // PeerConnectionObserver implementation.
  //

  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override{};
  void OnAddStream(webrtc::MediaStreamInterface* stream) override;
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;
  void OnDataChannel(webrtc::DataChannelInterface* channel) override {}
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override{};
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override{};
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnIceConnectionReceivingChange(bool receiving) override {}

  //
  // ConnectionClientObserver implementation.
  //

  virtual void OnSignedIn();
  virtual void OnDisconnected();
  virtual void OnPeerConnected(int id, const std::string& name);
  virtual void OnPeerDisconnected(int id);
  virtual void OnMessageFromPeer(int peer_id, const std::string& message);
  virtual void OnMessageSent(int err);
  virtual void OnServerConnectionFailure();

  //
  // MainWndCallback implementation.
  //

  virtual void StartLogin(const std::string& server, int port);
  virtual void DisconnectFromServer();
  virtual void ConnectToPeer(int peer_id);
  virtual void DisconnectFromCurrentPeer();
  virtual void UIThreadCallback(int msg_id, void* data);

  // CreateSessionDescriptionObserver implementation.
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
  virtual void OnFailure(const std::string& error);

 protected:
  // Send a message to the remote peer.
  void SendMessage(const std::string& json_object);

  int peer_id_;
  bool loopback_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  Connection* client_;
  MainWindow* main_wnd_;
  std::deque<std::string*> pending_messages_;
  std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
      active_streams_;
  std::string server_;
};

#endif  // __THROUGHNET_CONTROL_H__
