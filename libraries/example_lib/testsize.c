#include <exec/libraries.h>

#define SEGLISTPTR APTR
#include "example/examplebase.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("ExampleBase size: %d\n", (int) sizeof(struct ExampleBase));
    return 0;
}
