/*
*  Copyright 2016 The ThroughNet Project Authors. All rights reserved.
*
*  Ryan Lee (ryan.lee at throughnet.com)
*/

#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>
#include "throughnet.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

using namespace std;

bool parse_args(int argc, char* argv[], std::string& alias, std::string& connect_to, bool& server_mode);
void usage(const char* prg);
void read_stdin(Throughnet* tn, std::string id);
bool write_stdout(const char* buf, int len);
void set_mode(Throughnet* tn);
void ctrlc_handler(int s);

static Throughnet *tn_;


int main(int argc, char *argv[]) {

  string connec_to;
  string alias;
  bool server_mode;
  
  //
  // Parse arguments
  //

  if (!parse_args(argc, argv, alias, connec_to, server_mode)) {
    usage(argv[0]);
    return 1;
  }

  //
  // Set event handlers
  //

  Throughnet tn;
  set_mode(&tn);

  tn.On("signin", function_tn(Throughnet* tn, string id) {
    if (server_mode) {
      std::cerr << "Listening " << id << std::endl;
    }
    else {
      std::cerr << "Connecting to " << connec_to << std::endl;
      if (!server_mode) tn->Connect(connec_to);
    }
  });

  tn.On("connect", function_tn(Throughnet* tn, string id) {
    std::cerr << "Connected" << std::endl;
    std::thread(read_stdin, tn, id).detach();
  });

  tn.On("disconnect", function_tn(Throughnet* tn, string id) {
    if (server_mode)
      std::cerr << "Disconnected" << std::endl;
    else
      tn->SignOut();
  });

  tn.On("signout", function_tn(Throughnet* tn, string id) {
    Throughnet::Stop();
  });

  tn.On("error", function_tn(Throughnet* tn, string id){
    std::cerr << tn->GetErrorMessage() << std::endl;
    Throughnet::Stop();
  });

  tn.OnMessage(function_tn(Throughnet* tn, string id, Throughnet::Buffer& data) {
    if (!write_stdout(data.buf_, data.size_)) {
      tn->Disconnect(id);
    }
  });

  //
  // Sign in as anonymous user
  //

  tn.SignIn(alias, "anonymous", "nopassword");

  Throughnet::Run();
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

void read_stdin(Throughnet* tn, std::string id)
{
  int nbytes;
  char buf[32*1024];

  for (;;) {
    nbytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (nbytes <= 0) {
      tn->Disconnect(id);
      return;
    }

    if (!tn->SyncSend(id, buf, nbytes)) {
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
  tn_->SignOut();
}

void set_mode(Throughnet* tn)
{
  tn_ = tn;
  signal(SIGINT, ctrlc_handler);

#ifdef WIN32
  if (isatty(STDIN_FILENO)) setmode(STDIN_FILENO, O_TEXT);
  else setmode(STDIN_FILENO, O_BINARY);
  setmode(STDOUT_FILENO, O_BINARY);
#endif
}

bool parse_args(int argc, char* argv[], std::string& alias, std::string& connect_to, bool& server_mode) {
  if (argc == 2) {
    connect_to = argv[1];
    server_mode = false;
    return true;
  }
  else if (argc == 3 && std::string(argv[1]) == "-l") {
    alias = argv[2];
    server_mode = true;
    return true;
  }
  return false;
}

void usage(const char* prg) {
  std::cerr << "P2P netcat version 0.1 (http://github.com/throughnet/throughnet)" << std::endl << std::endl;
  std::cerr << "Usage: " << prg << " [-l] name" << std::endl << std::endl;
  std::cerr << "  Options:" << std::endl;
  std::cerr << "    -l      Listen mode, for inbound connections" << std::endl << std::endl;

  std::cerr << "Example: " << std::endl;
  std::cerr << "  > " << prg << " -l random_id      : Listen randoom_id" << std::endl;
  std::cerr << "  > " << prg << " random_id         : Connect to random_id" << std::endl;
}

