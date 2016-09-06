/*
*  Copyright 2016 The PeerConnect Project Authors. All rights reserved.
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
using namespace pc;

bool parse_args(int argc, char* argv[], string& local_channel, string& remote_channel, bool& server_mode);
void usage(const char* prg);
void read_stdin(PeerConnect* pc, std::string channel);
bool write_stdout(const char* buf, int len);
void set_mode(PeerConnect* pc);
void ctrlc_handler(int s);

static PeerConnect *pc_;


int main(int argc, char *argv[]) {

  string local_channel;
  string remote_channel;
  bool server_mode;
  
  //
  // Parse arguments
  //

  if (!parse_args(argc, argv, local_channel, remote_channel, server_mode)) {
    usage(argv[0]);
    return 1;
  }

  //
  // Set event handlers
  //

  PeerConnect pc(local_channel);

  set_mode(&pc);

  pc.On("open", function_pc( string peer ) {
    if (server_mode) {
      std::cerr << "Listening " << peer << std::endl;
    }
    else {
      std::cerr << "Connecting to " << remote_channel << std::endl;
      pc.Connect(remote_channel);
    }
  });

  pc.On("connect", function_pc( string peer ) {
    std::cerr << "Connected" << std::endl;
    std::thread(read_stdin, &pc, peer).detach();
  });

  pc.On("close", function_pc( string peer, CloseCode code, string desc ) {

    if ( peer == local_channel ) {
      PeerConnect::Stop();
    }
    else {
      pc.Close();
    }

    if ( !desc.empty() ) {
      std::cerr << desc << std::endl;
    }
  });

  pc.On("message", function_pc(string peer, PeerConnect::Buffer& data) {
    if (!write_stdout(data.buf_, data.size_)) {
      pc.Close(peer);
    }
  });

  //
  // Sign in as anonymous user
  //

  pc.Open();

  PeerConnect::Run();
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

void read_stdin(PeerConnect* pc, std::string channel)
{
  int nbytes;
  char buf[32*1024];

  for (;;) {
    nbytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (nbytes <= 0) {
      pc->Close( channel );
      return;
    }

    if (!pc->Send(channel, buf, nbytes, WAITING_ON)) {
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
  pc_->Close();
}

void set_mode(PeerConnect* pc)
{
  pc_ = pc;
  signal(SIGINT, ctrlc_handler);

#ifdef WIN32
  if (isatty(STDIN_FILENO)) setmode(STDIN_FILENO, O_TEXT);
  else setmode(STDIN_FILENO, O_BINARY);
  setmode(STDOUT_FILENO, O_BINARY);
#endif
}

bool parse_args(int argc, char* argv[], string& local_channel, string& remote_channel, bool& server_mode) {
  if (argc == 2) {
    remote_channel = argv[1];
    local_channel = PeerConnect::CreateRandomUuid();
    server_mode = false;
    return true;
  }
  else if (argc == 3 && std::string(argv[1]) == "-l") {
    local_channel = argv[2];
    server_mode = true;
    return true;
  }
  return false;
}

void usage(const char* prg) {
  std::cerr << "P2P netcat demo (http://github.com/peersio/peerconnect)" << std::endl << std::endl;
  std::cerr << "Usage: " << prg << " [-l] name" << std::endl << std::endl;
  std::cerr << "  Options:" << std::endl;
  std::cerr << "    -l      Listen mode, for inbound connections" << std::endl << std::endl;

  std::cerr << "Example: " << std::endl;
  std::cerr << "  > " << prg << " -l my_channel     : Listen my_channel" << std::endl;
  std::cerr << "  > " << prg << " my_channel        : Connect to my_channel" << std::endl;
}

