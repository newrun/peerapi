#include <stdio.h>
#include <stdlib.h>

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
