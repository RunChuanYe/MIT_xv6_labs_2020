#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int g(int x) {
  return x+3;
}

int f(int x) {
  return g(x);
}

void main(void) {
  printf("%d %d\n", f(8)+1, 13);

  // add the test code57616
  unsigned int i = 0x00646c72;
  printf("H%x wo%s", 57616, &i);
  // end add
  
  exit(0);
}