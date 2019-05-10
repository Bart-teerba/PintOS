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
  int fd;

  CHECK (mkdir ("a"), "mkdir \"a\"");
  CHECK (create ("a/b", 512), "create \"a/b\"");
  fd = wrap_open ("a");
  char name[1024];
  CHECK (readdir (fd, &name[0]), "readdir \"a\"");
  CHECK ((name[0] == 'b' && name[1] == '\0'), "name is \"b\"");
  CHECK (!readdir (fd, &name[0]), "readdir \"a\" (must fail)");
}
