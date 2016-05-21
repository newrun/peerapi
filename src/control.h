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

  explicit Control(ControlObserver* oberver, const std::string id);
  explicit Control(ControlObserver* oberver, const std::string id,
                   std::shared_ptr<Signal> signal);
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

  void SignIn();
  void Join(const std::string id);  // TODO: Rename to connect
  void Disconnect(const std::string id);
  void OnCommandReceived(const Json::Value& message);
  void OnSignalCommandReceived(const Json::Value& message);

  //
  // PeerObserver implementation
  //

  virtual bool SendCommand(const std::string& id, const std::string& command, const Json::Value& data);
  virtual void QueueDisconnect(const std::string id);
  virtual void OnConnected(const std::string id);
  virtual void OnDisconnected(const std::string id);
  virtual void OnPeerMessage(const std::string& id, const char* buffer, const size_t size);

  // implements the MessageHandler interface
  void OnMessage(rtc::Message* msg);

protected:
  void OnSignedIn(const Json::Value& data);
  void OnCreated(const Json::Value& data);
  void OnJoined(const Json::Value& data);
  void OnLeaved(const Json::Value& data);
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);
  void CreateOffer(const Json::Value& data);
  void AddIceCandidate(const std::string& peer_id, const Json::Value& data);
  void ReceiveOfferSdp(const std::string& peer_id, const Json::Value& data);
  void ReceiveAnswerSdp(const std::string& peer_id, const Json::Value& data);

  void DisconnectPeer(const std::string id);

  std::string id_;
  std::string session_id_;
  std::shared_ptr<Signal> signal_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;

  typedef rtc::scoped_refptr<PeerControl> Peer;
  std::map<std::string, Peer> peers_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

private:

  enum {
    MSG_COMMAND_RECEIVED,
    MSG_QUEUE_DISCONNECT,
    MSG_QUQUE_DISCONNECT_PEER
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
