#include <stdio.h>

int main(int argc, char **argv)
{
  printf("hello\n");
  printf("exec base is: shit\n");
  printf("hello: %04x\n", *((unsigned int *) 4));
  return 0;
}
