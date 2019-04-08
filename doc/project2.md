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

- Aqcuire lock and release lock before and after editing and accessing `ref_cnt` to make sure `ref_cnt` can only be edited by one thread at a time.


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
static void start_process (void *file_name_) 
{
    ...
    /* add file_deny_write() call . */
    ...
}
bool load (const char *file_name, void (**eip) (void), void **esp)
{ 
	... /* Set running thread's cur_file*/
}
void process_exit (void)
{
    ...	/* clean up all the file desccriptors that belong to current process). */
    ... /* get thread's cur_file and call add file_allow_write(). */
}
```

syscall.c

```c
/* Add global variable. */
struct lock *filesys_lock;

/* Add helper functions */
/* Find a usable fd number from current thread's fd_spots, create a fd_file_map, add it to current thread's fd_list */
int add_file(struct file* file)
    
/* remove the mapping struct according to fd in current thread's fd_list. Update the fd_spots array */
void remove_file(int fd); 

/* get the mapping struct according to fd from current thread's fd_list */
struct file* get_file(int fd); 




/* Modify. */
/* Add the corresponding file operation call according to the input system call number (SYS_OPEN, SYS_READ...). */
void syscall_handler (struct intr_frame *f UNUSED) 
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

thread.h

```c
/* Add struct fields. */
struct thread {
    ...
    struct file* cur_file;
    struct list* fd_list;
    int fd_spots[1024];
  	...
}
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

- File descriptor management: To correctly allocate and deallocate the file descriptor, for each process, we use an int array of length 1024 to record available file descriptor numbers to allocate. We use int field `1` to indicate the corresponding index is already used as an `fd`, and `0` meaning not being used. We initialize the array to be all zero, except for `arr[0]` and `arr[1]` having  `1` because they are reserved for `STDIN_FILENO` and `STDOUT_FILENO`. Then to find a usable `fd` to allocate, we simply iterate through the array until we find an empty spot, i.e. `0`, and set it to `1`. When we finish using the `fd` number, we set `arr[fd]` back to `0`. 1024 should be far more enough than the file descriptors we will possibly need. The above process is part of implementation inside `add_file` and `remove_file`. 

- Before any file operation call through the `syscall_handler`, we will have to call `validate_addr()` (implemented in task2) to validate the input argument address. If not valid, we simply exit the user process.
- After validating the arguments, we will have to obtain the global lock `filesys_lock` so that no multiple ﬁlesystem functions are called concurrently. After that, we should retrieve the arguments from `argv`, call the corresponding file operation functions according to the input system call number. If any of the call fails due to reasons such as invalid file descriptor, we should exit the user process. 
- Right before we return to user, we should release the global lock `filesys_lock`. To avoid  being verbose, we don't repeat this inside each function.
- The details of each function are as follows:
- `bool create (const char *file, unsigned initial_size)`: 

  - Call `filesys_create (const char *name, off_t initial_size)`. Return the return value in `success` .
- `bool remove (const char *file) `:
  - Call `filesys_remove (const char *name)`. Return the return value in `success` .
- `int open (const char *file)`

  - Call `filesys_open (const char *name)`. 
  - Check the returned `file*` pointer, if it's `NULL`, return `-1` , meaning failure. 
  - Otherwise, we should call `add_file(struct file* file)`, which find the next usable file descriptor in `fd_spots` and the returned `file *` to initialize a `struct fd_file_map`, and add it to current thread's `fd_list`, which stores the mapping between file descriptors and `file *` per thread. Return the used `fd` .
- `int filesize (int fd)`

  - Use `struct file* get_file(int fd)` to Iterate through the current thread's `fd_list`, if no such `fd` is found, -1 is returned.
  - If found, we retrieve the corresponding `struct file *` to call `file_length (struct file *)`. Return the returned value.
- `int read (int fd, void *buffer, unsigned size) `

  - use `validate_addr()` to validate the address at  `*buff` and `*(buff + size)`. If not valid, return -1.
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

- We choose to use an int array to allocate file descriptors. We thought about using a single int, but this will not allow us to reuse the number we used before, which means the number will continue to increase over time of use, and since fd is represented as int type, this might lead to overflow. That's why we choose to implement the former. 

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

**Answer**
- name: "main"
- address: `0xc000e000`

`pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000ee0c "\210\357", priority = 31, allelem = {prev = 0xc0034b50 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0034b60 <ready_list>, next = 0xc0034b68 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}`

`pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0034b58<all_list+8>}, elem = {prev = 0xc0034b60 <ready_list>, next = 0xc0034b68 <ready_list+8>}, pagedir =0x0, magic = 3446325067}`

<br>

2. What is the backtrace for the current thread? Copy the backtrace from gdb as your answer and also copy down the line of c code corresponding to each function call.

**Answer**

`#0  process_execute (file_name=file_name@entry=0xc0007d50 "args-none") at ../../userprog/process.c:32`

`#1  0xc002025e in run_task (argv=0xc0034a0c <argv+12>) at ../../threads/init.c:288`

`#2  0xc00208e4 in run_actions (argv=0xc0034a0c <argv+12>) at ../../threads/init.c:340`

`#3  main () at ../../threads/init.c:133`

<br>

3. Set a breakpoint at `start_process` and continue to that point. What is the name and address of the thread running this function? What other threads are present in pintos at this time? Copy their `struct threads`.

**Answer**
- name: "args-none"
- address: `0xc010a000`

`pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_BLOCKED, name = "main", '\000' <repeats 11 times>, stack = 0xc000eebc "\001", priority = 31, allelem = {prev = 0xc0034b50 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0036554 <temporary+4>, next = 0xc003655c <temporary+12>}, pagedir = 0x0, magic = 3446325067}`

`pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc010a020}, elem = {prev = 0xc0034b60 <ready_list>, next = 0xc0034b68 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}`

`pintos-debug: dumplist #2: 0xc010a000 {tid = 3, status = THREAD_RUNNING, name = "args-none\000\000\000\000\000\000", stack = 0xc010afd4 "", priority = 31, allelem = {prev = 0xc0104020, next = 0xc0034b58 <all_list+8>}, elem = {prev = 0xc0034b60 <ready_list>, next = 0xc0034b68 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}`


<br>

4. Where is the thread running `start_process` created? Copy down this line of code.

**Answer**

It is created at line 45 in `process_execute` function in `process.c`.
```c
tid_t process_execute(const char *file_name) {
  ...
  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
  ...
}
```

gdb reference:
`#0  start_process (file_name_=0xc0109000) at ../../userprog/process.c:55`
`#1  0xc002128f in kernel_thread (function=0xc002a125 <start_process>, aux=0xc0109000) at ../../threads/thread.c:424`
`#2  0x00000000 in ?? ()`
<br>

5. Continue one more time. The userprogram should cause a page fault and thus cause the page fault handler to be executed. It’ll look something like

```
[Thread <main>] #1 stopped.

pintos-debug: a page fault exception occurred in user mode 
pintos-debug: hit ’c’ to continue, or ’s’ to step to intr_handler 0xc0021ab7 in intr0e_stub ()
```

Please ﬁnd out what line of our user program caused the page fault. Don’t worry if it’s just an hex address. (Hint: `btpagefault` may be useful)


**Answer**

- address: `0x0804870c`

<br>


6. The reason why btpagefault returns an hex address is because pintos-gdb build/kernel.o only loads in the symbols from the kernel. The instruction that caused the page fault is in our userprogram so we have to load these symbols into gdb. To do this use

`loadusersymbols build/tests/userprog/args-none`

. Now do `btpagefault` again and copy down the results.


**Answer**

`#0  _start (argc=<error reading variable: can't compute CFA for this frame>, argv=<error reading variable: can't compute CFA for this frame>) at ../../lib/user/entry.c:9`

<br>


7. Why did our user program page fault on this line?


**Answer**

The system has problem finding `argc` and `argv`. This is because argument passing has not been implemented yet.

<br>