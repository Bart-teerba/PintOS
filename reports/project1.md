Final Report for Project 1: Threads
===================================

## Group Members

- Yifan Ning <yifanning@berkeley.edu>
- Zhi Chen <zhichen98@berkeley.edu>
- Jerry Li <jerry.li.cr@berkeley.edu>
- Bart Ba <bart98@berkeley.edu>
<br />



## Changes in Code


### Task 1: Eﬃcient Alarm Clock

<br />

#### thread.h
```c
void thread_sleep_foreach (thread_action_func *, void *aux);       /* Apply func for all threads in sleep_list */
/* Combined the two funcitons */
/* Reason: higher performance
 * have to halt in the while loop in thread_sleep_foreach because we are using a sorted sleep_list
 * the functionality of thread_sleep_foreach is limitedly defined */
```

- Merged `thread_donate_priority` with `thread_update_priority` because the ready_list is no longer sorted and it does not have to resort after a thread’s priority change.
- `thread_priority_less` remains unchanged, but now it serves for  `list_max` instead of `list_insert_ordered` because  `ready_list` is no longer sorted.


#### thread.c
```c
/* Comparator to sort sleep_list depending on wakeTick, uses list_insert_ordered */
bool thread_sleeper_more(const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...}
/* Changed the name into thread_sleeper_less to reflect the actual implementation */

/* Func for thread_sleep_foreach, can call thread_unblock */
void unblock_check (struct thread *t, void *aux){...};
/* This function been merged with thread_sleep_foreach, as discussed in the thread.h */

In function yield_create()
/* Added a thread_yield at the end of thread_create */
/* Reason: to reschedule after unblock a thread */

/* Insert thread into sleep_list with thread_sleeper_more, because we want to put small elements in the front */
thread_block (void){...}
/* Checked a thread's wake_tick before inserting it into sleep_list
 * threads blocked by timer_sleep have wake_tick >= 0
 * others have wake_tick == -1 
*/
/* Reason: Avoid putting a thread that is blocked  
 * elsewhere into sleep_list, since it is not sleeping */

/* Pop thread from sleep_list */
thread_unblock (void){...}
/* Same logic as above, when removing a thread from sleep_list */
/* re-init a thread's wake_tick as -1 to show that it has been wakened */

```

In `thread_block` and `thread_unblock`, we use different logics for threads which are blocked by `timer_sleep` and those who are not. This is because we don’t want to put threads which are not blocked by `timer_sleep` into `sleep_list` which might cause a thread which should be blocked be wakened by `thread_sleep_foreach`.



<br />


### Task 2: Priority Scheduler

<br />

#### In thread.h
- Merged `thread_donate_priority` with `thread_update_priority` because the ready_list is no longer sorted and it does not have to resort after a thread’s priority change.
- `thread_priority_less` remains unchanged, but now it serves for  `list_max` instead of `list_insert_ordered` because  `ready_list` is no longer sorted.
  
#### In sync.h
- `lock_priority_less` and  `cond_sema_priority_less`  , both of which take in two `list_elem` and `aux`.
- Now those two functions serve for  `list_max` instead of `list_insert_ordered` because  thread’s `locks`  and cond’s `waiters` are  no longer sorted
  
#### In thread.c
- Keep all lists unsorted except for `sleep_list`. The insertion occurs in `thread_block`.
  
#### In sync.c
- `&sema→waiters` is no longer sorted. Meanwhile,  `list_max` using `cond_sema_priority_less` and then remove the corresponding `sema_elem` instead of `list_pop_front` directly.  
- Changed the name of `lock_acquire` to `thread_get_lock`
- In `cond_signal`
  - Simply get the max `sema_elem` using `list_max` with `cond_sema_priority_less` instead of sorting the whole list.
  - Pop the `sema_elem` from the list.


#### Section 5 -- Priority Scheduling for semaphores and locks:
- To implement this, we just have to sort `&sema->waiters` in `sema_up` according to `cond_sema_priority_less` function, which compares the `priority_effective` of the corresponding thread of two semas.
  - As explained above, use `list_max` and `list_remove` instead of sorting the whole list.
  
  
#### Section 6 -- Priority Scheduling for condition variables:

- Similar to Section 5, we just have to sort `&cond->waiters` in `cond_signal` according to `cond_sema_priority_less` function, which compares the `priority_effective` of the corresponding thread of two conditional variables.
  - As explained above, use `list_max` and `list_remove` instead of sorting the whole list.


<br />


### Task 3: Multi-level Feedback Queue Scheduler (MLFQS)

<br />

- Put refresh calls (`refresh_priority(), refresh_recent_cpu...`) inside `timer.c` for 2 reasons:
  - A series of function calls need to use the `ticks` variable inside `timer.c`.
  - The priority maintenance should be done by CPU, so it makes more sense to put the calls inside `timer.c` instead of `threads.tick()` of `threads`

<br />

## Reﬂection
 
-  A reﬂection on the project – what exactly did each member do? What went well, and what could be improved?

<br />

Generally, this is a collaborative team where all members are proactive. Members who finished early, like Richard, helped other tasks to debug. 

Divided team into three parts, each part takes charge of one task. Depending on the complexity of code, each part contains different number of members. Task 1 is taken by Bart and Jerry, Task 2 is taken by Jerry and Eric, and Task 3 is taken by Richard. 

#### The project is divided into four stages:
1. Preparation: including reading the case, finding and sharing available resources, divid work and go through the schedule
2. Design Doc write up: each part write up individually and then merge it by going through together
3. Coding: individually so the whole project works, then merge by going through together
3. Reflection and moving forward: host a meeting that reflects on the last project while kicking off the coming project

#### Good traits:
1. Limited the number of coders: this is made possible by a high fidelity of design; when coding up, all members are in one physical location
2. Great internal communication

#### Improvements:
1. Need to start earlier so the team will have more time to teach each other the tasks they are responsible for
2. Should spend more time on connecting the project to lecture content so it enhances the effectiveness of studying
3. Better coordination of time and location
