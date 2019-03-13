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

<br />

### 2. Algorithms
<br />

### 3. Synchronization


<br />

### 4. Rationale


<br />
<br />
<br />
<br />


## Task 2: Process Control Syscalls


<br />

### 1. Data structures and functions



<br />

### 2. Algorithms



<br />

### 3. Synchronization




<br />

### 4. Rationale



<br />
<br />
<br />
<br />


## Task 3: File Operation Syscalls

<br />

### 1. Data structures and functions

<br />

### 2. Algorithms




<br />

### 3. Synchronization




<br />

### 4. Rationale




<br />
<br />
<br />
<br />

## Additional Questions



1. Take a look at the Project 2 test suite in pintos/src/tests/userprog. Some of the test cases will intentionally provide invalid pointers as syscall arguments, in order to test whether your implementation safely handles the reading and writing of user process memory. Please identify a test case that uses an invalid stack pointer ($esp) when making a syscall. Provide the name of the test and explain how the test works. (Your explanation should be very speciﬁc: use line numbers and the actual names of variables when explaining the test case.)



2. Please identify a test case that uses a valid stack pointer when making a syscall, but the stack pointer is too close to a page boundary, so some of the syscall arguments are located in invalid memory. (Your implementation should kill the user process in this case.) Provide the name of the test and explain how the test works. (Your explanation should be very speciﬁc: use line numbers and the actual names of variables when explaining the test case.)



3. Identify one part of the project requirements which is not fully tested by the existing test suite. Explain what kind of test needs to be added to the test suite, in order to provide coverage for that part of the project. (There are multiple good answers for this question.)



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