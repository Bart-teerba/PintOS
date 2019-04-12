/* This tests the remove under normal situation: Create a file, 
   remove it. Open it, which must fail. */

#include <stdio.h>
#include <syscall.h>
#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
	create ("remove.txt", 0);
    remove("remove.txt");
	int handle = open ("remove.txt");
	if (handle != -1) 
		fail ("I successfully opened a removed file with handle %d", handle);			
}
