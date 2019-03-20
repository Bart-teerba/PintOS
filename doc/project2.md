Design Document for Project 2: User Programs
======================================

## Group Members

* Yifan Ning <yifanning@berkeley.edu>
* Zhi Chen <zhichen98@berkeley.edu>
* Jerry Li <jerry.li.cr@berkeley.edu>
* Bart Ba <bart98@berkeley.edu>

<br />
<br />
<br />
<br />

## Task 1: Argument Passing

<br />

### 1. Data structures and functions


**process.c**

- Initialize three new variables in `load` function: `argv`, `argc`, `addresses`.
- - When parsing `file_name` and spliting it into arguments as `char*` by space, store the arguments in `argv`.
- - Store the total number of arguments as `argc`.
- - Update addresses when `memset` arguments in memory. 

```c
bool load (const char *file_name, void (**eip) (void), void **esp) {
  ...
  /* Addition */
  
  char** argv;              /* Stores the pointers of arguments as char*. */
  int argc;                 /* Stores the total number of arguments. */
  char* addresses;          /* Stores the addresses of arguments. */
  ...
}
```
<br />

### 2. Algorithms


- After initializing three new variables in `load`, parse `file_name` into `argv` using `strtok_r()`. At the same time, increment `argc` from 0 while parsing `file_name` so that `argc` equals the total number of arguments. Note that `argv` has maximum size 256.
- Set `file_name` as `argv[0]` which is the name of excutable if it exists.
- After `setup_stack(esp)`, use while loop to `memset` arguments stored in `argv` backward, and decrease `esp` at each step. At same time, store these `esp` into `addresses`.
- After storing all arguments in memory, use `word_align` to round `sp` downward to a number of multiple of 4.
- Before storing the addresses of arguments in memory, first `memset` 0 to represent the end of `argv`. Then, `memset` addresses stored in `addresses` backward, which are the addresses of arguments stored in `argv`.
- Store `argv[0]`'s address as `argv` in stack, which is current `sp + 4`.
- Store `argc`.
- Store 0 as "return address" and set `esp` equal to the address of "return address".

<br />

### 3. Synchronization


There is no synchronization issues.

<br />

### 4. Rationale


- Change `file_name` in `load` so that there is no problem passing argument between different functions like `process_execute`, `start_process` and `load`. 
- Parse `file_name` before `filesys_open` so that we open the correct excutable file whose name does not contain any argument.
- Store arguments and addresses after `setup_stack` so that it is made sure that the page has already been set up.
- Follow the instruction in "3.1.9" when setting memory so that the memory is set correctly and the stack pointer is at return address.


<br />
<br />
<br />
<br />


## Task 2: Process Control Syscalls


<br />

### 1. Data structures and functions


**thread.h**
```c
/* Addition */

/* Stores the wait status of a thread. Can be accessed by its parent. */
struct wait_status {
  struct list_elem elem;            /* 'children' list element. */
  struct lock lock;                 /* Protects ref_cnt. */
  int ref_cnt;                      /* 2 = child and parent both alive, 
                                       1 = either child or parent alive, 
                                       0 = child and parent both dead. */
  tid_t tid;                        /* Child thread id. */        
  int exit_code;                    /* Child exit code, if dead. */
  struct semaphore dead;            /* 0 = child alive,
                                       1 = child dead. */
};

struct thread {
  ...
  struct wait_status *wait_status;    /* This process’s wait state. */
  struct list children;               /* A list of wait status of children. */
  bool load_success;            /* True = load successfully,
                                         False = load unsuccessfully. */
  semaphore child_load_sema;          /* Semaphore to make sure child has finished loading */
  semaphore parent_check_load_sema;   /* Semaphore to make sure parent has viewed child's loading status. */
  ...
}

/* find a thread with given tid and return its thread pointer. */
struct thread *get_thread_by_tid(tid_t tid);

/* init function for wait_status struct. */
void wait_status_init (struct wait_status *, tid_t tid);
```

**thread.c**
```c
/* Addition */

/* find a thread with given tid and return its thread pointer. */
struct thread *get_thread_by_tid(tid_t tid){...}

/* init function for wait_status struct. */
void wait_status_init (struct wait_status *ws, tid_t tid){
  lock_init (&ws->lock);
  ws->ref_cnt = 2;
  ws->tid = tid;
  ws-exit_code = NULL;
  sema_init (&ws->dead, 0);
}

/* Modification */

static void init_thread (struct thread *t, const char *name, int priority) {
  ...
  wait_status_init(&t->wait_status, t->tid);
  list_init(&t->children);
  sema_init(&t->child_load_sema, 0)
  sema_init(&parent_check_load_sema, 0)
  ...
}
```

**userprog/syscall.c**
```c
/* Modification */

/* Handle system call functions and use validate_addr to check if the address is valid. */
static void syscall_handler (struct intr_frame *f UNUSED); 

/* Addition */

/* If the address is valid, return it. Otherwise exit(1). */
void *validate_addr (void *ptr); 
```

**process.c**
```c
/* Modification */
tid_t process_execute (const char *file_name) {...}
static void start_process (void *file_name_) {...}
int process_wait (tid_t child_tid) {...}
process_exit (void) {...}
```

**exception.c**
```c
kill(struct intr_frame *f);       /* sema_up() sema in wait_status before thread_exit(). */
```

<br />

### 2. Algorithms

**Preparation**

**halt**

- In `syscall_handler`, call `shutdown_power_off()` to terminate Pintos if `args[0] == SYS_HALT`.

**practice**

- In `syscall_handler`, set `f->eax` as `args[1] + 1` if `args[0] == SYS_PRACTICE`.

**exec**

In `process_execute`

- `sema_down` `child_load_sema` in child thread. 
- After `sema_down`, check `load_success` which represent whether a child has been loaded successfully or not. 
- If it is `True`, return `tid`. Else, return `-1`. 
- Before returning, `sema_up` `parent_check_load_sema` in child thread to wake it up. 
- Push child's `wait_status` to `children` list in the current thread if the child has been successfully loaded. 
- Child thread can be acquired by function `get_thread_by_tid` implemented in `thread.c`.

In `start_process`

- Set `load_success` as success after `load` has finished. 
- Then `sema_up` `child_load_sema` in the current thread to wake its parent up. 
- Immediately, `sema_down` `parent_check_load_sema` to make sure the parent has checked child's load status before move on. 

**wait**

In `process_wait`

- Iterate through `children` list in current thread and look for the thread which has the same tid as the one provided.
- If no thread can match the given tid, return -1
- If the thread exists, `sema_down` `dead` in child's `wait_status`.
- After `sema_down`, init a new local variable `exit_code` equal to `exit_code` in child's `wait_status`. 
- Acquire `lock` in child's `wait_status` and decrease `ref_cnt` in child's `wait_status` by 1 following by a `release_lock`. 
- If child's `ref_cnt` in `wait_status` equal to 0, free child's `wait_status` and remove `elem` which represents child's `wait_status` from list. 
- Return local variable `exit_code`.

**exit vs wait**

In `process_exit`

- Acquire `lock` in `wait_status` of current thread and decrease `ref_cnt` in `wait_status` of current thread by 1. Don't release lock here.
- If `ref_cnt` in `wait_status` of current thread equal to 0, release lock, free its `wait_status` and remove `elem` which represents its `wait_status` from list. Else, just release lock.
- Iterate through `children` list in current thread.
- For each child thread, acquire `lock` in its `wait_status` and decrease `ref_cnt` in its `wait_status` by 1. Don't release lock here. 
- If child's `ref_cnt` in `wait_status` equal to 0, release lock, free child's `wait_status` and remove `elem` which represents child's `wait_status` from list. Else, just release lock.
- `sema_up` `dead` in current thread if the current `wait_status` still exists.


<br />

### 3. Synchronization

**exec**

- Two semaphores `child_load_sema` and `parent_check_load_sema` are designed to synchronize `process_execute` (which runs in the parent thread) and `start_process` (which runs in the child thread). 
- - `child_load_sema` is to make sure that the logic check of child's load status in parent's `process_execute` happens after loading child thread in `start_process`.
- - `parent_check_load_sema` is to make sure that parent process can still get child's `load_success` even if child process has a higher priority than parent thread.

- Aqcuire lock and release lock before and after editing `ref_cnt` to make sure `ref_cnt` can only be edited by one thread at a time.


<br />

### 4. Rationale

For wait:

- If parent exit first: parent would decrease all its children's `ref_cnt` by 1 to represent that it has exited already. If a child thread's `ref_cnt` is 0, it means that both of its parent and itself has exited. Thus, we free its `wait_status`.

- If child exit first: child would decrease its `ref_cnt` by 1 to represent that it has exited already. It also store its exit code in its `wait_status.exit_code`. Thus, its parent can access its exit code later when calling wait on it.

- If a parent wait for a child while a child is still running: it has to `sema_down` child's `dead` sema first when child's `dead` sema is only `sema_up`ed when it exits. Thus it makes sure that the parent thread will wait until the child thread finishes.

<br />
<br />
<br />
<br />


## Task 3: File Operation Syscalls

<br />

### 1. Data structures and functions

process.c

```c
/* Modify. */
bool load (const char *file_name, void (**eip) (void), void **esp)
{
    ...
    /* add file_deny_write() call . */
    ...
}
void
process_exit (void)
{
    ... /* get thread's cur_file and call add file_allow_write(). */
}
```

syscall.c

```c
/* Add global variable. */
struct lock *filesys_lock;
int fd_spots[4096];

/* Add helper functions */
/* Find a usable fd number from fd_spots, create a fd_file_map, add it to current thread's fd_list */
int add_file(struct file* file)
    
/* remove the mapping struct according to fd in current thread's fd_list. Update the fd_spots array */
void remove_file(int fd); 

/* get the mapping struct according to fd from current thread's fd_list */
struct file* get_file(int fd); 




/* Modify. */
/* Add the corresponding file operation call according to the input system call number (SYS_OPEN, SYS_READ...). */
syscall_handler (struct intr_frame *f UNUSED) 
{
	...    
}

/* Implement. See algorithms for details. */
bool create (const char *file, unsigned initial_size) {...};
bool remove (const char *file) {...};
int open (const char *file) {...};
int filesize (int fd) {...};
int read (int fd, void *buffer, unsigned size) {...};
int write (int fd, const void *buffer, unsigned size) {...};
void seek (int fd, unsigned position) {...};
unsigned tell (int fd) {...};
void close (int fd) {...};
```

thread.c

```c
/* Add */
void set_thread_cur_file (file*);
```

thread.h

```c
/* Add struct fields. */
struct thread {
    ...
    struct file* cur_file;
    struct list* fd_list;
  	...
}

/* Set thread's cur_file to file* */
void set_thread_cur_file (file*);
```

process.h

```c
/* Add struct. */
struct fd_file_map { 
    int fd;  
    struct file * file;    
}
```



<br />

### 2. Algorithms

- Before any file operation call through the `syscall_handler`, we will have to call `validate_addr()` (implemented in task2) to validate the input argument address. If not valid, we simply return -1 to user.
- After validating the arguments, we will have to obtain the global lock `filesys_lock` so that no multiple ﬁlesystem functions are called concurrently. After that, we should retrieve the arguments from `argv`, call the corresponding file operation functions according to the input system call number. 
- Note: Right before we return to user, we should release the global lock `filesys_lock`. To avoid  being verbose, we don't repeat this inside each function.
- The details of each functions are as follows:
- `bool create (const char *file, unsigned initial_size)`: 

  - Call `filesys_create (const char *name, off_t initial_size)`. Return the return value in `success` to user.
- `bool remove (const char *file) `:

  - Call `filesys_remove (const char *name)`. Return the return value in `success` to user.
- `int open (const char *file)`

  - Call `filesys_open (const char *name)`. 
  - Check the returned `file*` pointer, if it's `NULL`, return `-1` to users, meaning failure. 
  - Otherwise, we should call `add_file(struct file* file)`, which find the next usable file descriptor in `fd_spots` and the returned `file *` to initialize a `struct fd_file_map`, and add it to current thread's `fd_list`, which stores the mapping between file descriptors and `file *` per thread. Return the used `fd` to user.
- `int filesize (int fd)`

  - Use `struct file* get_file(int fd)` to Iterate through the current thread's `fd_list`, if no such `fd` is found, -1 is returned to user.
  - If found, we retrieve the corresponding `struct file *` to call `file_length (struct file *)`. Return the returned value to user.
- `int read (int fd, void *buffer, unsigned size) `

  - use `validate_addr()` to validate the address at  `*buff` and `*(buff + size)`. If not valid, return -1 to user.
  - If `fd == 0`, we use `uint8_t input_getc (void)` inside `src/devices/input.c` to repeatedly get characters and put into our buffer until we reach number of `size` . Return number of bytes we read to user. 
  - If `fd != 0`, retrieve the corresponding `struct file*` by calling `struct file* get_file(int fd)` on `fd`. If not found, return -1 to user. If found, using retrieved `struct file*`, call `file_read (struct file *file, void *buffer, off_t size)`, return the returned value to user. 
- `int write (int fd, const void *buffer, unsigned size) `
  - Use `validate_addr()` to validate the address at  `*buff` and `*(buff + size)`. If not valid, return -1 to user.
  - if `fd == 1`, we use `void putbuf (const char *buffer, size_t n)` inside `src/lib/kernel/console.c` to write buff to console in one shot, unless the size is too big and we break up it into several calls. Return number of bytes we actually write to user.
  - if `fd != 1`, retrieve the corresponding `struct file*` by calling `struct file* get_file(int fd)` on `fd`. If not found, return -1 to user. If found, using retrieved `struct file*`, call `off_t
     file_write (struct file *file, const void *buffer, off_t size)`. Return the returned value to user. 
- `void seek (int fd, unsigned position) `
  - Retrieve the corresponding `struct file*` by calling `struct file* get_file(int fd)` on `fd`. If not found, stop here. If found, using retrieved `struct file*`, call `void
    file_seek (struct file *file, off_t new_pos)`. Nothing is returned to user. 
- `unsigned tell (int fd) `r
  - Retrieve the corresponding `struct file*` by calling `struct file* get_file(int fd)` on `fd`. If not found, return 0. If found, using retrieved `struct file*`, call `off_t
    file_tell (struct file *file)`. Return the returned value to user. 
- `void close (int fd)`

  - Retrieve the corresponding `struct file*` by calling `struct file* get_file(int fd)` on `fd`. If not found, stop here. If found, using retrieved `struct file*`, call `void
    file_close (struct file *file)`. Then, deallocate the `fd` by `calling remove_file(int fd)`. 

<br />

### 3. Synchronization

- We use a global lock `filesys_lock` to ensure thread-safe file operation syscalls. In order to make a file operation syscall, a thread has to acquire the global lock first, and will only able to go forward if the global lock has been released by any other threads doing file operations. 
- To prevent any modification on the executable on disk when a user processing is running, we call `file_deny_write` immediately when the process starts running, i.e. after load success, and call `file_allow_write` at the end of process termination. The arguments for these two calls can be retrieved through current running thread's `struct file* cur_file` field.


<br />

### 4. Rationale

- Since hash map is discouraged (according to piazza), we use a `struct` to maintain a mapping between `fd` and `struct file*`, which is added to the `fd_list` when one mapping struct is created.
- We have to check all the addresses provided by user are valid using `valiadate_addr`. This includes the addresses in `argv`, buffer address and buffer address + size in `read`, `write`.

- To correctly allocate the file descriptor, we use a global int array of length 4096. We use `1` to indicate the corresponding index is already used as an `fd`, and `0` meaning not being used. We initialize the array to be all zero, except for `arr[0]` and `arr[1]`,which is `1` because they are reserved for `STDIN_FILENO` and `STDOUT_FILENO`. Then to find a usable `fd` to allocate, we simply iterate through the array until we find an empty spot, i.e. `0`, and set it to `1`. When we finish using the `fd` number, we then set `arr[fd]` back to `0`. 4096 should be far more enough than the file descriptors we will possibly need. We feel this method is both efficient and easy to code. 

- We define helper function `add_file`, `get_file`, `remove_file` to reduce the unnecessary repeated code segments. 


<br />
<br />
<br />
<br />

## Additional Questions



1. Take a look at the Project 2 test suite in pintos/src/tests/userprog. Some of the test cases will intentionally provide invalid pointers as syscall arguments, in order to test whether your implementation safely handles the reading and writing of user process memory. Please identify a test case that uses an invalid stack pointer ($esp) when making a syscall. Provide the name of the test and explain how the test works. (Your explanation should be very speciﬁc: use line numbers and the actual names of variables when explaining the test case.)

**Answer**

Test case: `sc-bad-sp.c`

Problem: In line 18, a negative virtual address which lies approximately 64MB below the code segment is assigned to esp. Because this address is not mapped to any valid memory, the test process must be terminated with -1 exit code.

<br>



2. Please identify a test case that uses a valid stack pointer when making a syscall, but the stack pointer is too close to a page boundary, so some of the syscall arguments are located in invalid memory. (Your implementation should kill the user process in this case.) Provide the name of the test and explain how the test works. (Your explanation should be very speciﬁc: use line numbers and the actual names of variables when explaining the test case.)

**Answer**

Test case: `sc-bad-arg.c`

Problem: In line 14, the top boundary of stack which is `0xbffffffc` is assigned to esp. When the system call number of `SYS_EXIT` is stored at this address, the argument to the `SYS_EXIT` would be above the top of the user address space. When `SYS_EXIT` is invoked, the test process should be terminated with -1 exit code.






3. Identify one part of the project requirements which is not fully tested by the existing test suite. Explain what kind of test needs to be added to the test suite, in order to provide coverage for that part of the project. (There are multiple good answers for this question.)

**Answer**

There are no tests written for some of the file operations. For example, `remove`, `seek`, `tell`, etc. We should add tests to test their functionality works as expected.  For example, for `remove`, we should create a file, remove the file we created, and try to open the file. Failure is expected. 



4. Answer the GDB questions below.

To be sucessful at Pintos, a strong familiarity with GDB is extrememly helpful. As such, in this design doc, we will ask some questions about pintos which can be answered with GDB. Please make sure that each member of your group does this section of the design doc. (Feel free to work together of course!)

We will use GDB on the ﬁrst test of project 2 – args-none. To get started please ﬁrst build project 2 by running make check in the src/userprog directory. Then start Pintos with the args-none test by doing

```
pintos --gdb --filesys-size=2 -p build/tests/userprog/args-none -a args-none \ 
-- -q -f run args-none
```



Finally, start GDB (`pintos-gdb build/kernel.o`) and attach it to the Pintos process (`debugpintos`). If any of these instructions are not clear, please take another look at the GDB section of Project 1’s spec.

The questions you should answer in your design doc are the following.

1. Set a break point at `process_execute` and continue to that point. What is the name and address of the thread running this function? What other threads are present in pintos at this time? Copy their `struct threads`. (Hint: for the last part `dumplist &all_list thread allelem` may be useful.)
2. What is the backtrace for the current thread? Copy the backtrace from gdb as your answer and also copy down the line of c code corresponding to each function call.
3. Set a breakpoint at `start_process` and continue to that point. What is the name and address of the thread running this function? What other threads are present in pintos at this time? Copy their `struct threads`.
4. Where is the thread running `start_process` created? Copy down this line of code.
5. Continue one more time. The userprogram should cause a page fault and thus cause the page fault handler to be executed. It’ll look something like

```
[Thread <main>] #1 stopped.

pintos-debug: a page fault exception occurred in user mode 
pintos-debug: hit ’c’ to continue, or ’s’ to step to intr_handler 0xc0021ab7 in intr0e_stub ()
```

Please ﬁnd out what line of our user program caused the page fault. Don’t worry if it’s just an hex address. (Hint: `btpagefault` may be useful)

6. The reason why btpagefault returns an hex address is because pintos-gdb build/kernel.o only loads in the symbols from the kernel. The instruction that caused the page fault is in our userprogram so we have to load these symbols into gdb. To do this use

`loadusersymbols build/tests/userprog/args-none`

. Now do `btpagefault` again and copy down the results.

7. Why did our user program page fault on this line?

