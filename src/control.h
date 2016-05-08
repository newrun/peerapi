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

  explicit Control(ControlObserver* oberver, const std::string channel);
  explicit Control(ControlObserver* oberver, const std::string channel,
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

  bool Send(const char* buffer, const size_t size, const std::string *peer_id = nullptr);

  void SignIn();
  void OnCommandReceived(const Json::Value& message);
  void OnSignalCommandReceived(const Json::Value& message);

  //
  // PeerObserver implementation
  //

  virtual bool SendCommand(const std::string& command, const Json::Value& data, const std::string& peer_sid);
  virtual void OnConnected(const std::string peer_id);
  virtual void OnData(const std::string& peer_id, const char* buffer, const size_t size);

  // implements the MessageHandler interface
  void OnMessage(rtc::Message* msg);

  //
  // sigslots
  //

  sigslot::signal2<const std::string&, const std::string&> SignalOnConnected_;
  sigslot::signal4<const std::string&, const std::string&, const char*, const size_t> SignalOnData_;

  const std::string& channel_name() { return channel_name_; }


protected:
  void OnSignedIn(const Json::Value& data);
  void OnJoined(const Json::Value& data);
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);
  void CreateOffer(const Json::Value& data);
  void AddIceCandidate(const std::string& peer_sid, const Json::Value& data);
  void ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data);
  void ReceiveAnswerSdp(const std::string& peer_sid, const Json::Value& data);

  std::string channel_name_;
  std::string session_id_;
  std::shared_ptr<Signal> signal_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;

  typedef rtc::scoped_refptr<PeerControl> Peer;
  std::map<std::string, Peer> peers_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

private:

  enum {
    MSG_COMMAND_RECEIVED
  };

  struct ControlMessageData : public rtc::MessageData {
    explicit ControlMessageData(Json::Value data) : data_(data) {}
    Json::Value data_;
  };

  rtc::Thread* signaling_thread_;
  ControlObserver* observer_;

};

} // namespace tn

#endif // __THROUGHNET_CONTROL_H__
