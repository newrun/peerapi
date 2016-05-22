/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#ifndef __THROUGHNET_CONTROL_H__
#define __THROUGHNET_CONTROL_H__

#include <memory>

#include "peer.h"
#include "signalconnection.h"
#include "controlobserver.h"

#include "webrtc/base/sigslot.h"
#include "fakeaudiocapturemodule.h"


namespace tn {

class Control
    : public PeerObserver,
      public sigslot::has_slots<>,
      public rtc::MessageHandler {
public:
  typedef std::vector<rtc::scoped_ptr<PeerDataChannelObserver> >
            DataChannelList;

  explicit Control();
  explicit Control(std::shared_ptr<Signal> signal);
  ~Control();

  //
  // Initialize and release
  //

  bool InitializeControl();
  void DeleteControl();
  
  //
  // Negotiation and send/emit data
  //

  void Send(const std::string to, const char* buffer, const size_t size);

  void SignIn(const std::string& user_id, const std::string& user_password, const std::string& open_id);
  void Connect(const std::string id);
  void Disconnect(const std::string id);
  void OnCommandReceived(const Json::Value& message);
  void OnSignalCommandReceived(const Json::Value& message);

  //
  // PeerObserver implementation
  //

  virtual void SendCommand(const std::string& id, const std::string& command, const Json::Value& data);
  virtual void QueuePeerDisconnect(const std::string id);
  virtual void OnPeerConnected(const std::string id);
  virtual void OnPeerDisconnected(const std::string id);
  virtual void OnPeerMessage(const std::string& id, const char* buffer, const size_t size);

  // Register/Unregister observer
  void RegisterObserver(ControlObserver* observer);
  void UnregisterObserver();

  // implements the MessageHandler interface
  void OnMessage(rtc::Message* msg);

protected:
  void CreateChannel(const std::string name);
  void JoinChannel(const std::string name);
  void LeaveChannel(const std::string name);
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);
  void CreateOffer(const Json::Value& data);
  void AddIceCandidate(const std::string& peer_id, const Json::Value& data);
  void ReceiveOfferSdp(const std::string& peer_id, const Json::Value& data);
  void ReceiveAnswerSdp(const std::string& peer_id, const Json::Value& data);
  void DisconnectPeer(const std::string id);

  void OnSignedIn(const Json::Value& data);
  void OnChannelCreated(const Json::Value& data);
  void OnChannelJoined(const Json::Value& data);
  void OnChannelLeaved(const Json::Value& data);

  // open_id_: other peers can find this peer by open_id_ and it is user_id or alias
  // user_id_: A user id to sign in signal server (could be 'anonymous' for guest user)
  // session_id_: A unique id for signal server connection
  std::string open_id_;
  std::string user_id_;
  std::string session_id_;

  std::shared_ptr<Signal> signal_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;

  typedef rtc::scoped_refptr<PeerControl> Peer;
  std::map<std::string, Peer> peers_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

private:

  enum {
    MSG_COMMAND_RECEIVED,       // Command has been received from signal server
    MSG_QUEUE_DISCONNECT,       // Queue disconnection request (+subsequent peer disconnection)
    MSG_QUEUE_PEER_DISCONNECT   // Queue peer disconnection request
  };

  struct ControlMessageData : public rtc::MessageData {
    explicit ControlMessageData(Json::Value data) : data_(data) {}
    explicit ControlMessageData(const std::string data) : data_string_(data) {}
    Json::Value data_;
    std::string data_string_;
  };

  rtc::Thread* webrtc_thread_;
  ControlObserver* observer_;

};

} // namespace tn

#endif // __THROUGHNET_CONTROL_H__
