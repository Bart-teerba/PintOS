#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
int create (const char *file, unsigned initial_size);
int remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

/* Add global variable. */
struct lock *filesys_lock;


void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  uint32_t* args = ((uint32_t*) f->esp);
  //printf("System call number: %d\n", args[0]);
  if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    thread_exit();
  } else if (args[0] == SYS_HALT) {
    //printf("System power off\n");
    shutdown_power_off();
  } else if (args[0] == SYS_PRACTICE) {
    f->eax = args[1] + 1;
    //printf("Practice: %d + 1 = %d\n", args[1], args[1] + 1);
  } else if (args[0] == SYS_EXEC) {
    f->eax = args[1];
    //printf("Execute: %d\n", args[1]);
  } else if (args[0] == SYS_WAIT) {
    f->eax = args[1];
    //printf("Wait status: %d\n", args[1]);
  } else if (args[0] == SYS_CREATE) {

  } else if (args[0] == SYS_REMOVE) {

  } else if (args[0] == SYS_OPEN) {

  } else if (args[0] == SYS_FILESIZE) {

  } else if (args[0] == SYS_READ) {

  } else if (args[0] == SYS_WRITE) {
    int fd = args[1];
    void * buff = args[2];
    size_t size = args[3];
    write(fd, buff, size);
  } else if (args[0] == SYS_SEEK) {

  } else if (args[0] == SYS_TELL) {

  } else if (args[0] == SYS_CLOSE) {

  }

}

int create (const char *file, unsigned initial_size) 
{

};  

int remove (const char *file) {

};

int open (const char *file) {

};

int filesize (int fd) {

};

int read (int fd, void *buffer, unsigned size) {


};

int write (int fd, const void *buffer, unsigned size) 
{
  if (fd == 1) {
    putbuf (buffer, size);
  }
}

void seek (int fd, unsigned position) {

};

unsigned tell (int fd) {

};

void close (int fd) {

};