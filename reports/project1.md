Final Report for Project 1: Threads
===================================

## Group Members

- Yifan Ning <yifanning@berkeley.edu>
- Zhi Chen <zhichen98@berkeley.edu>
- Jerry Li <jerry.li.cr@berkeley.edu>
- Bart Ba <bart98@berkeley.edu>

<br />

## Task 1: Eﬃcient Alarm Clock

<br />

-  the changes you made since your initial design document and why you made them (feel free to re-iterate what you discussed with your TA in the design review)



## Task 2: Priority Scheduler

<br />

-  the changes you made since your initial design document and why you made them (feel free to re-iterate what you discussed with your TA in the design review)





## Task 3: Multi-level Feedback Queue Scheduler (MLFQS)

<br />

-  the changes you made since your initial design document and why you made them (feel free to re-iterate what you discussed with your TA in the design review)
  - We actually put refresh calls (`refresh_priority(), refresh_recent_cpu...`) inside `timer.c` mainly for 2 reasons:
    - The series of function calls need to use the `ticks` variable inside `timer.c`.
    - The priority maintenance should be done by cpu, so it makes more sense to put the calls inside `timer.c` instead of `threads.tick()` of `threads`



##Reﬂection

-  A reﬂection on the project – what exactly did each member do? What went well, and what could be improved?
  - 
  - 
  - Yifan Ning: I am mainly responsible for implementing task3, code style check and add explanation docs. After I finished my part, I also helped my partners debug task1 and task2.