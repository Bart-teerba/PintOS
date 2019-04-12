/* This tests the filesize and write: Create a file with size 10, open it,
   write 15 bytes to it. Since that exceeds the size of file we created,
   we should expect both write() and filesize() return 10. */

#include <stdio.h>
#include <syscall.h>
#include "tests/userprog/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void)
{
	create ("filesize.txt", 10);
	int fd = open ("filesize.txt");
	char *buff =  "abcdefghijklmnopqrst";
	int bytes_written = write(fd, buff, 15);
	int size_returned = filesize (fd);
	if (bytes_written != 10 || size_returned != 10 ) 
	    fail ("Bytes written (%d) different than filesize (%d)", bytes_written, size_returned);
}
