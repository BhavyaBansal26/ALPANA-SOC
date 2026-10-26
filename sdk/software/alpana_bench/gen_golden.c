#define HOST
#include <stdio.h>
#include "stress_kern.h"
#include "stress_list.h"
int main(void){
  printf("/* generated on host by gen_golden.c - do not edit */\n");
#define G(name, call) printf("#define GOLD_%s 0x%08xu\n", #name, call);
  STRESS_TESTS(G)
  return 0;
}
