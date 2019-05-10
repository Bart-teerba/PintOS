/* Tests isdir(). */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

static int
wrap_open (const char *name)
{
  static int fds[8], fd_cnt;
  int fd, i;

  CHECK ((fd = open (name)) > 1, "open \"%s\"", name);
  for (i = 0; i < fd_cnt; i++)
    if (fds[i] == fd)
      fail ("fd returned is not unique");
  fds[fd_cnt++] = fd;
  return fd;
}

void
test_main (void)
{
  int fd1;
  int fd2;

  CHECK (mkdir ("a"), "mkdir \"a\"");
  CHECK (create ("b", 512), "create \"b\"");
  fd1 = wrap_open ("a");
  fd2 = wrap_open ("b");
  CHECK (isdir (fd1), "isdir \"a\"");
  CHECK (!isdir (fd2), "isdir \"b\" (must fail)");
}
