/*
*  Copyright 2016 The PeerApi Project Authors. All rights reserved.
*
*  Ryan Lee
*/

#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <string>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/uio.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include "peerconnect.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

using namespace std;


bool parse_args(int argc, char* argv[], string& local_peer_id, string& remote_peer_id, bool& server_mode);
void usage(const char* prg);
void read_stdin(Peer* peer, std::string peer_id);
bool write_stdout(const char* buf, int len);
void set_mode(Peer* peer);
void ctrlc_handler(int s);

static Peer *peer_;


int main(int argc, char *argv[]) {

  string local_peer_id;
  string remote_peer_id;
  bool server_mode;
  
  //
  // Parse arguments
  //

  if (!parse_args(argc, argv, local_peer_id, remote_peer_id, server_mode)) {
    usage(argv[0]);
    return 1;
  }

  //
  // Set event handlers
  //

  Peer peer(local_peer_id);

  set_mode(&peer);

  peer.On("open", function_peer( string peer_id ) {
    if (server_mode) {
      std::cerr << "Listening " << peer_id << std::endl;
    }
    else {
      std::cerr << "Connecting to " << remote_peer_id << std::endl;
      peer.Connect(remote_peer_id);
    }
  });

  peer.On("connect", function_peer( string peer_id ) {
    std::cerr << "Connected" << std::endl;
    std::thread(read_stdin, &peer, peer_id).detach();
  });

  peer.On("close", function_peer( string peer_id, CloseCode code, string desc ) {

    if ( peer_id == local_peer_id ) {
      Peer::Stop();
    }
    else {
      peer.Close();
    }

    if ( !desc.empty() ) {
      std::cerr << desc << std::endl;
    }
  });

  peer.On("message", function_peer(string peer_id, char *data, size_t size) {
    if (!write_stdout(data, size)) {
      peer.Close(peer_id);
    }
  });

  //
  // Sign in as anonymous user
  //

  peer.Open();

  Peer::Run();
  return 0;
}



#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

void read_stdin(Peer* peer, std::string peer_id)
{
  int nbytes;
  char buf[32*1024];

  for (;;) {
    nbytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (nbytes <= 0) {
      peer->Close( peer_id );
      return;
    }

    if (!peer->Send(peer_id, buf, nbytes, SYNC_ON)) {
      return;
    }
  }
}

bool write_stdout(const char* buf, int len)
{
  int nbytes;
  int remain = len;

  for (;;) {
    nbytes = write(STDOUT_FILENO, buf, len);
    if (nbytes <= 0) {
      return false;
    }
    remain -= nbytes;
    if (remain <= 0) break;
  }

  return true;
}

void ctrlc_handler(int s) {
  std::cerr << "Terminating..." << std::endl;
  peer_->Close();
}

void set_mode(Peer* peer)
{
  peer_ = peer;
  signal(SIGINT, ctrlc_handler);

#ifdef WIN32
  if (isatty(STDIN_FILENO)) setmode(STDIN_FILENO, O_TEXT);
  else setmode(STDIN_FILENO, O_BINARY);
  setmode(STDOUT_FILENO, O_BINARY);
#endif
}

bool parse_args(int argc, char* argv[], string& local_peer_id, string& remote_peer_id, bool& server_mode) {
  if (argc == 2) {
    remote_peer_id = argv[1];
    local_peer_id = Peer::CreateRandomUuid();
    server_mode = false;
    return true;
  }
  else if (argc == 3 && std::string(argv[1]) == "-l") {
    local_peer_id = argv[2];
    server_mode = true;
    return true;
  }
  return false;
}

void usage(const char* prg) {
  std::cerr << "P2P netcat demo (http://github.com/peerborough/peerapi)" << std::endl << std::endl;
  std::cerr << "Usage: " << prg << " [-l] name" << std::endl << std::endl;
  std::cerr << "  Options:" << std::endl;
  std::cerr << "    -l      Listen mode, for inbound connections" << std::endl << std::endl;

  std::cerr << "Example: " << std::endl;
  std::cerr << "  > " << prg << " -l peer_name     : Listen peer_name" << std::endl;
  std::cerr << "  > " << prg << " peer_name        : Connect to peer_name" << std::endl;
}

