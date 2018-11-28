#include <stdio.h>
#include "unity.h"


void PORT_iassert_fun(const char *msg, int line) {

  printf("ASSERT in file %s:%d", msg, line);
  TEST_ASSERT(0);
}
