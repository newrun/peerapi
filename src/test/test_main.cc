#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <throughnet.h>

using namespace std;

#define CHECK_ERR(err, msg) { \
    if (err != 0) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

/* ===========================================================================
 * Test connect_peer1() with valid condition
 */

void connect_peer1()
{
  int err = 0;

  Throughnet(function_tn(string event_id, tn::EventData& data){
    if (event_id == "connected") {
      this_->Send("your-guid", "Hello");
    }
  })
  .Connect("my-guid", "{}");
    

  CHECK_ERR(err, "nothing");
}


/* ===========================================================================
 * Usage:  example
 */

int main(int argc, char *argv[])
{
  printf("===== Test started.   =====\n");

  connect_peer1();

  printf("===== Test completed. =====\n");
  return 0;
}
