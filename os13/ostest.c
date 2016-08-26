#include <stdio.h>

int main(int argc, char **argv)
{
  printf("hello\n");
  printf("Exec Base: %04x\n", *((unsigned int *) 4));
  return 0;
}
