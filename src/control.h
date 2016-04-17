/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#ifndef __THROUGHNET_CONTROL_H__
#define __THROUGHNET_CONTROL_H__

#include "webrtc/base/sigslot.h"
#include "fakeaudiocapturemodule.h"

#include "peer.h"
#include "signalconnection.h"


namespace tn {

class Control
    : public PeerObserver,
      public sigslot::has_slots<>,
      public rtc::RefCountInterface {
public:
  typedef std::vector<rtc::scoped_ptr<PeerDataChannelObserver> >
            DataChannelList;

  explicit Control(const std::string channel);
  explicit Control(const std::string channel,
                   rtc::scoped_refptr<Signal> signal);
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
  void OnSignedIn(const std::string& sid);
  void OnCommandReceived(const std::string& message);

  //
  // PeerObserver implementation
  //

  virtual bool SendCommand(const std::string& command, const Json::Value& data, const std::string& peer_sid);
  virtual void OnConnected(const std::string peer_id);
  virtual void OnData(const std::string& peer_id, const char* buffer, const size_t size);

  //
  // sigslots
  //

  sigslot::signal2<const std::string&, const std::string&> SignalOnConnected_;
  sigslot::signal4<const std::string&, const std::string&, const char*, const size_t> SignalOnData_;

  const std::string& channel_name() { return channel_name_; }


protected:
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);
  void CreateOffer(const Json::Value& data);
  void AddIceCandidate(const std::string& peer_sid, const Json::Value& data);
  void ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data);
  void ReceiveAnswerSdp(const std::string& peer_sid, const Json::Value& data);

  std::string channel_name_;
  std::string session_id_;
  rtc::scoped_refptr<Signal> signal_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;

  typedef rtc::scoped_refptr<PeerControl> Peer;
  std::map<std::string, Peer> peers_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;

private:

};

} // namespace tn

#endif // __THROUGHNET_CONTROL_H__
