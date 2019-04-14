1. # Final Report for Project 2: User Programme

   ## Group Members

   - Yifan Ning <yifanning@berkeley.edu>
   - Zhi Chen <zhichen98@berkeley.edu>
   - Jerry Li <jerry.li.cr@berkeley.edu>
   - Bart Ba <bart98@berkeley.edu>
     <br />



   ## Changes in Code

   ### Task 1: 

   <br />

   - Update child thread's name as `argv[0]` in ```load``` since the original `file_name` contains not only the name of the executable but also the arguments.

   <br />

   ### Task 2: 

   <br />

   - Only one semaphore is needed in thread struct to synchronize the load status between child and parent thread. Rationale explained below.
   - Do not need `get_thread_by_tid` method in `thread.c`.
   - Do not init `thread->wait_status` in `init_thread`. Instead, malloc and init `thread->wait_status` in `start_process` if the process is successfully loaded.
   - Create a helper function `wait_status_helper` to deal with `wait_status` in `process_wait` and `process_exit`. In `wait_status_helper`, before checking it's `ref_cnt` and deciding whether to free it or not, check if it is `null` since `wait_status` is only malloced when the process is successfully loaded.
   - In `process_exit`, only `sema_up` `wait_status->dead` if `wait_status` has not been freed.
   - Rationale of load synchronization:
     - In `process_execute`, pass a struct called `argStruct` which stores `file_name` and parent thread pointer to `start_process` instead of file_name `solely`.
     - After loading child process in `start_process`, store the load status into `parent->load_success` and `sema_up` parent thread's `child_load_sema`. 
     - In `process_execute`, `sema_down` current thread's `child_load_sema` before returning and check its`load_status`.
     - Even if the child process has finished, we can still have the child process's load status since it is stored in the parent thread.



   <br />

   ### Task 3:

   <br />

   - Instead of using an array to allocate file descriptors, we simply use an int to store `next_fd_to_allocate`, since we don't have that much memory.
   - We actually put struct `fd_file_map` inside `thread.h` instead of `process.h` so it's easier to access.

   <br />

   ### Student Testing Report:

   <br />

   - my-test-1:

     - This test tests the remove file system call implementation under normal situation. 

     - The general mechanism is: we first create a file (assuming create works since we have other tests for it), then remove it. Next time we open the file with same file name, it should fail, so that open call should return us -1. 

     - my-test-1.output

       ```
       Copying tests/userprog/my-test-1 to scratch partition...
       qemu -hda /tmp/Qrmt1XkJbk.dsk -m 4 -net none -nographic -monitor null
       PiLo hda1
       Loading...........
       Kernel command line: -q -f extract run my-test-1
       Pintos booting with 4,088 kB RAM...
       382 pages available in kernel pool.
       382 pages available in user pool.
       Calibrating timer...  419,020,800 loops/s.
       hda: 5,040 sectors (2 MB), model "QM00001", serial "QEMU HARDDISK"
       hda1: 176 sectors (88 kB), Pintos OS kernel (20)
       hda2: 4,096 sectors (2 MB), Pintos file system (21)
       hda3: 102 sectors (51 kB), Pintos scratch (22)
       filesys: using hda2
       scratch: using hda3
       Formatting file system...done.
       Boot complete.
       Extracting ustar archive from scratch device into file system...
       Putting 'my-test-1' into the file system...
       Erasing ustar archive...
       Executing 'my-test-1':
       (my-test-1) begin
       (my-test-1) end
       my-test-1: exit(0)
       Execution of 'my-test-1' complete.
       Timer: 57 ticks
       Thread: 0 idle ticks, 56 kernel ticks, 1 user ticks
       hda2 (filesys): 105 reads, 215 writes
       hda3 (scratch): 101 reads, 2 writes
       Console: 886 characters output
       Keyboard: 0 keys pressed
       Exception: 0 page faults
       Powering off...
       ```

     - my-test-1.result

       ```
       PASS
       ```

     - - If the remove is not correctly implemented or not implemented at all, then the file name will still point to a valid file when we call open, so open will succeed, fail will be called and terminal will output FAIL.
       - If open is not correctly implemented, for example, opening a non-exist file does not return us -1 but 0, then the terminal will output FAIL.

   - my-test-2:

     - This tests the functionality of  filesize and write.

     - The general mechanism is: we create a file with size 10, open it, write 15 bytes to it. Since that exceeds the size of file we created, we should expect both write() and filesize() return 10.

     - my-test-2.output

       ```
       - Copying tests/userprog/my-test-2 to scratch partition...
       qemu -hda /tmp/aFgYok99k6.dsk -m 4 -net none -nographic -monitor null
       PiLo hda1
       Loading...........
       Kernel command line: -q -f extract run my-test-2
       Pintos booting with 4,088 kB RAM...
       382 pages available in kernel pool.
       382 pages available in user pool.
       Calibrating timer...  209,510,400 loops/s.
       hda: 5,040 sectors (2 MB), model "QM00001", serial "QEMU HARDDISK"
       hda1: 176 sectors (88 kB), Pintos OS kernel (20)
       hda2: 4,096 sectors (2 MB), Pintos file system (21)
       hda3: 103 sectors (51 kB), Pintos scratch (22)
       filesys: using hda2
       scratch: using hda3
       Formatting file system...done.
       Boot complete.
       Extracting ustar archive from scratch device into file system...
       Putting 'my-test-2' into the file system...
       Erasing ustar archive...
       Executing 'my-test-2':
       (my-test-2) begin
       (my-test-2) end
       my-test-2: exit(0)
       Execution of 'my-test-2' complete.
       Timer: 56 ticks
       Thread: 0 idle ticks, 55 kernel ticks, 1 user ticks
       hda2 (filesys): 88 reads, 216 writes
       hda3 (scratch): 102 reads, 2 writes
       Console: 885 characters output
       Keyboard: 0 keys pressed
       Exception: 0 page faults
       Powering off...
       
       ```

     - my-test-2.result

       ```
       PASS
       ```

     - - If the filesize is not correctly implemented or not implemented at all, then the filesize call will not return us 10, then fail will be called and terminal will output FAIL.
       - If write is not correctly implemented, for example, writing past the end of file actually extends  the file until all bytes are written, then byte_written will be more than 10, then fail will be called and terminal will output FAIL .

   - The overall experience is pretty good, I feel there are actually lots of tests that can be added, and writing the tests actually make me rethink about the decision choices we make. There are also useful examples and functions we can call for testing. 

   - For testing system improvement, I feel we can have better output formatting, right now there's no space separating each test, making it hard to recognize the border of each test. 

   - Lessons we learn: It's somewhat hard to write test to include everything. For one thing, writing tests takes time. For another thing, there are always edge cases one can't think of. But it's definitely useful and necessary, as it helps reduce bugs and make you rethink the decision choices you make. This itself is a way of making one better at coding.

   <br />

   ## Reﬂection

   - A reﬂection on the project – what exactly did each member do? What went well, and what could be improved?



   #### Good traits:

   1. All members are clear about the task they are responsible for and worked actively.
   2. Great internal communication.

   #### Improvements:

   1. Need to start earlier since this time we used two slipper days.
   2. Should read and understand the tests more carefully.
   3. Consider more details when writing the design doc.