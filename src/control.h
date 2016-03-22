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
#include "signal.h"


#define WAIT_(ex, timeout)                     \
  do {                                              \
    uint32_t start = rtc::Time();                   \
    while (!(ex) && rtc::Time() < start + timeout) { \
      rtc::Thread::Current()->ProcessMessages(1);   \
    }                                               \
  } while (0)


namespace tn {

class Control
    : public PeerObserver,
      public sigslot::has_slots<>,
      public rtc::RefCountInterface {
public:
  typedef std::vector<rtc::scoped_ptr<PeerDataChannelObserver> >
            DataChannelList;

  static void Connect(Control* caller,
                      Control* callee);

  explicit Control(const std::string channel);
  explicit Control(const std::string channel,
                   rtc::scoped_refptr<Signal> signal);
  ~Control();

  bool InitializeControl();
  void DeleteControl();
  bool Send(const std::string& message, const std::string *peer_id = nullptr);

  const std::string& channel_name() { return channel_name_; }


  void SignIn();
  void OnSignedIn(const std::string& sid);
  void OnOfferPeer(const std::string& peer_sid);
  void OnAnswerPeer(const std::string& peer_sid);
  void OnCommandReceived(const std::string& message);
  void OnPeerOpened(const std::string& peer_sid);
  void OnPeerMessage(const webrtc::DataBuffer& buffer);

  //
  // PeerObserver implementation
  //

  virtual bool SendCommand(const std::string& command, const Json::Value& data, const std::string& peer_sid);
  virtual void OnConnected(const std::string peer_id);
  virtual void OnData(const std::string& peer_id, const char* buffer, const size_t size);




  void TestWaitForConnection(uint32_t kMaxWait);
  void TestWaitForChannelOpen(uint32_t kMaxWait);
  void TestWaitForMessage(const std::string& message, uint32_t kMaxWait);
  void TestWaitForClose(uint32_t kMaxWait);
  bool CheckForConnection();


  // sigslots
  sigslot::signal2<const std::string&, const std::string&> SignalOnConnected_;
  sigslot::signal4<const std::string&, const std::string&, const char*, const size_t> SignalOnData_;
//  sigslot::signal1<std::string*> SignalOnIceCandidateCreated;
//  sigslot::signal3<const std::string&,
//                   int,
//                   const std::string&> SignalOnIceCandidateReady;
//  sigslot::signal1<std::string*> SignalOnSdpCreated;
//  sigslot::signal1<const std::string&> SignalOnSdpReady;
//  sigslot::signal1<webrtc::DataChannelInterface*> SignalOnDataChannel;

protected:
  bool CreatePeerFactory(const webrtc::MediaConstraintsInterface* constraints);

  void AddIceCandidate(const std::string& peer_sid, const Json::Value& data);
  void ReceiveOfferSdp(const std::string& peer_sid, const Json::Value& data);
  void ReceiveAnswerSdp(const std::string& peer_sid, const Json::Value& data);


  std::string channel_name_;
  std::string device_id_;
  std::string session_id_;
  rtc::scoped_refptr<Signal> signal_;
  rtc::scoped_refptr<FakeAudioCaptureModule> fake_audio_capture_module_;

  typedef rtc::scoped_refptr<PeerControl> Peer;
  std::map<std::string, Peer> peers_;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  rtc::scoped_ptr<PeerDataChannelObserver> local_data_channel_;
  rtc::scoped_ptr<PeerDataChannelObserver> remote_data_channel_;
};

} // namespace tn

#endif // __THROUGHNET_CONTROL_H__
