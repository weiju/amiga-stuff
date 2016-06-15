#include <exec/types.h>
#include <exec/memory.h>

#include <example/example.h>

#include <proto/exec.h>
#include <proto/example.h>

#include <stdio.h>
#include <stdlib.h>

struct ExampleBase *ExampleBase = NULL;

int main(int argc, char **argv)
{
    ExampleBase = (APTR) OpenLibrary("example.library", 37);
    if (ExampleBase) {
        EXF_TestRequest("Test Message", "It works!", "OK");
        CloseLibrary((APTR) ExampleBase);
        exit(0);
    }
    printf("\nLibrary opening failed\n");
    exit(20);
}
