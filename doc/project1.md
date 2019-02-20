Design Document for Project 1: Threads
======================================

## Group Members

* Yifan Ning <yifanning@berkeley.edu>
* Zhi Chen <zhichen98@berkeley.edu>
* Jerry Li <jerry.li.cr@berkeley.edu>
* Bart Ba <bart98@berkeley.edu>



## Task 1: Eﬃcient Alarm Clock


## Task 1: Efficient Alarm Clock
#### 1. Data Structures and functions
timer.c
```c
/* Addition */
/* List of sleeping threads */
static struct list sleep_list;
/* Comparator to sort sleep_list depending on wakeTick, uses list_insert_ordered */

bool sleeper_comparator(const struct thread *thread1, const struct thread *thread2, void *aux UNUSED);
```

```c
/* Modification based on the original code */
void timer_init(void); /* Initialize a sleep_list */

void timer_sleep (int64_t ticks); /* Modify timer */
static void timer_interrupt (struct intr_frame *args UNUSED);  /* Modify timer_interrupt to make operations atomic */
```

thread.c

```c
/* Modification */
struct thread {
	...
	int wake_tick; /* Amount of time before wake */
	...
}
```

```c
static void init_thread (struct thread *t, const char *name, int priority) {
	...
	t->wake_tick = -1;
	...
}
```


#### 2. Algorithms
 - `time_sleep()`: Calculate the value of current + sleep to save as `until` that represents when the current thread will wake up. Then place the current thread to `sleep_list`. Call `thread_block()` to put itself to sleep, which changes the thread status to "THREAD_BLOCKED". Switch the context to the scheduler by calling `thread_yield()`. 
 - `timer_interrupt()`: this function pops a new thread in the `sleep_list` and increment the current time tick. In each call, it wakes the front of the `sleep_list`, if a thread's wakeup time is earlier than current time. In this case, we call `thread_unblock()` on the corresponding thread so it goes to `ready_list` and changes status to `THREAD_READY`. We keep doing this until the beginning thread of `sleep_list` has a wakeTick larger than current tick or it is empty.


#### 3. Synchronization
This plan makes sure that there's always only one thread being added to the `sleep_list` even when multiple threads call this function at the same time. Such synchronization is made sure by sharing a `sleep_list` across threads. 


#### 4. Rationale
* To minimize the amount of time spent in interrupt, this plan keeps a sleep list that is sorted based on the amount of sleep time. Such approach is going to make the the interrupt handler run as fast as possible. 
* `sleep_list` becomes a part of a critical section. This makes it atomic and makes sure that there's always only one thread being added at a time. 
* `timer_interrupt()` is also supposed to be atomic, but we don't have to put it into a critical section because it runs on CPU
* Assert the input tick value to make sure it falls within a plausible bound
* To make sure these executions run atomically and be consistent with OS behaviors, we disable interrupt before entering the following: `thread_unblock()`, `sleep_list_insert()`, `sleep_list_remove()`


## Task 2: Priority Scheduler



### 1. Data structures and functions

We redefine the original ``` priority ``` variable in thread as ``` priority_effective ```.

*In thread.h*

Add a lock pointer ``` waiting_lock ``` which keeps track of a lock which is acquired by the thread. <br />
Add a list ``` locks ``` which stores all the needed locks. <br />
Add a integer ``` priority_ori ``` which stores the original priority of the thread. This should not be influenced by donation and it can only be changed when init and set priority. <br />
Add a function ``` thread_get_lock ``` which takes in a lock pointer.<br />
Add a function ``` thread_rm_lock ``` which takes in a lock pointer.<br />
Add a function ``` thread_donate_priority ``` which takes in a thread pointer.<br />
Add a function ``` thread_update_priority ``` which takes in a thread pointer.<br />
Add a function ``` thread_priority_less ``` which is fed into ``` list_insert_ordered ``` funciton of list. <br />

```c
/* Add three new variables in thread. */
struct thread
{
  struct lock *waiting_lock;          /* a lock acuired by the thread. */
  struct list locks;                  /* stores all the needed locks */
  int pritority_ori;                  /* stores the original priority of the thread */
  ...
}

void thread_get_lock (struct lock *);
void thread_rm_lock (struct lock *);
void thread_donate_priority (struct thread *);
void thread_update_priority (struct thread *);
bool priority_less(const struct list_elem *e1, const struct list_elem *e2, void *aux);
```


*In sync.h*

in struct lock, we add two variables:

``` struct list_elem elem ```, which makes it posible to store lock in a list.<br />
``` int priority_max ```, which keeps track of the highest priority of the current lock. <br />

We also have to add two funcitons which are served as argument of ``` list_insert_ordered ``` function:

``` lock_priority_less ``` and ``` cond_sema_priority_less ```, both of which take in two ``` list_elem ``` and ``` aux ```.

```c
/* Add two variables in struct lock. */
struct lock
  {
    struct list_elem elem;          /* elem to store in list */
    int max_priority;               /* highest priority of threads which needs this lock */
  };
  
/* Add two less functions */
bool lock_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux);
bool cond_sema_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux);
```



*In thread.c*

Change all the ``` list_push_back ``` into ``` list_insert_ordered ``` with ``` thread_priority_less ``` function. There are three methods which involve this amendment: ``` thread_unblock ```, ``` thread_yield ```, ``` init_thread ```.

We have to change ``` thread_set_priority ``` funciton. Here, we have to update the ``` priority_ori ``` as ``` new_pritority ``` and ensure the ``` priority_effective ``` variable of the thread is the max of ``` priority_ori ``` and itself. If ``` priority_effective ``` is changed, we should yield the current thread.

In ``` init_thread ```, we have to consider the new variable added to thread including: ``` priority_ori ```, ``` locks ```, ``` waiting_lock ```. 

Add a function ``` thread_get_lock ``` which takes in a lock pointer. It inserts a lock into thread's ``` locks ``` list. At the same time, update thread's priority if possible.

Add a function ``` thread_rm_lock ``` which takes in a lock pointer. It removes a lock from thread's ``` locks ``` list. At the same time, update thread's priority if possible.

Add a function ``` thread_donate_priority ``` which takes in a thread pointer. It should call ``` thread_update_priority ``` to update the thread's priority. After calling ``` thread_update_priority ```, we should consider the case that the thread's priority has been changed and we should update its position in ``` ready_list ```.

Add a function ``` thread_update_priority ``` which takes in a thread pointer. We update thread's priority as the max of the largest priority of its ``` locks ``` and it's ``` priority_effective ```. We should not consider the old ``` priority_effective ``` value since we might call this function after we remove a lock which might cause the ``` priority_effective ``` to decrease. 

Add a function ``` thread_priority_less ``` which is fed into ``` list_insert_ordered ``` funciton of list.

```c
void thread_get_lock (struct lock *){...};
void thread_rm_lock (struct lock *){...};
void thread_donate_priority (struct thread *){...};
void thread_update_priority (struct thread *){...};
bool priority_less(const struct list_elem *e1, const struct list_elem *e2, void *aux){...};
```


*In sync.c* 

In ``` sema_up ```, we have to sort ``` &sema->waiters ```.

In ``` lock_acquire ```, which has to handle recursive donation. We designed to store ``` priority_max ``` in lock and then update threads' ``` priority_effective ``` according to ``` priority_max ``` of locks it has. In ``` lock_acquire ```, we should update ``` priority_max ``` according to the ``` priority_effective ``` of current thread. Use recursion to visit ``` waiting_lock ``` of all layers of threads. After we update all the locks, we sema_down the lock. After getting current_thread, we should clear the ``` lock_waiting ``` variable of this thread and set lock's ``` priority_max ``` as the ``` priority_effective ``` of this thread. Finally, we let the thread get lock by calling ``` thread_hold_the_lock ```. 

In ``` lock_release ```, which has to add a part to remove the lock in the lock list of the corresponding thread by calling ``` thread_remove_lock ```. 

In ``` cond_signal ```, we have to sort ``` &cond->waiters ``` by ``` cond_sema_priority_less ```.

We create two functions ``` lock_priority_less ``` and ``` cond_sema_priority_less ``` which should compare ``` priority_max ``` of two locks and ``` priority_effective ``` of threads corresponding to semaphores or condition variables. 

```c
bool lock_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...};
bool cond_sema_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...};
```




### 2. Algorithms

This is where you tell us how your code will work. Your description should be at a level below the high level description of requirements given in the assignment. We have read the project spec too, so it is unnecessary to repeat or rephrase what is stated here. On the other hand, your description should be at a level above the code itself. Don’t give a line-by-line run-down of what code you plan to write. Instead, you should try to convince us that your design satisﬁes all the requirements, including any uncommon edge cases.

The length of this section depends on the complexity of the task and the complexity of your design. Simple explanations are preferred, but if your explanation is vague or does not provide enough details, you will be penalized. Here are some tips:





### 3. Synchronization

Describe your strategy for preventing race conditions and convince us that it works in all cases. Here are some tips for writing this section:

• This section should be structured as a list of all potential concurrent accesses to shared resources. For each case, you should prove that your synchronization design ensures correct behavior.

• An operating system kernel is a complex, multithreaded program, in which synchronizing multiple threads can be diﬃcult. The best synchronization strategies are simple and easily veriﬁable, which leaves little room for mistakes. If your synchronization strategy is diﬃcult to explain, consider how you could simplify it.

• You should also aim to make your synchronization as eﬃcient as possible, in terms of time and memory.

• Synchronization issues revolve around shared data. A good strategy for reasoning about synchronization is to identify which pieces of data are accessed by multiple independent actors (whether they are threads or interrupt handlers). Then, prove that the shared data always remains consistent.

• Lists are a common cause of synchronization issues. Lists in Pintos are not thread-safe.

• Do not forget to consider memory deallocation as a synchronization issue. If you want to use pointers to struct thread, then you need to prove those threads can’t exit and be deallocated while you’re using them.

• If you create new functions, you should consider whether the function could be called in 2 threads at the same time. If your function access any global or static variables, you need to show that there are no synchronization issues.

• Interrupt handlers cannot acquire locks. If you need to access a synchronized variable from an interrupt handler, consider disabling interrupts.

• Locks do not prevent a thread from being preempted. Threads can be interrupted during a critical section. Locks only guarantee that the critical section is only entered by one thread at a time.



### 4. Rationale

Tell us why your design is better than the alternatives that you considered, or point out any shortcomings it may have. You should think about whether your design is easy to conceptualize, how much coding it will require, the time/space complexity of your algorithms, and how easy/diﬃcult it would be to extend your design to accommodate additional features.



## Task 3: Multi-level Feedback Queue Scheduler (MLFQS)

### 1. Data structures and functions

Write down any struct deﬁnitions, global (or static) variables, typedefs, or enumerations that you will be adding or modifying (if it already exists). These deﬁnitions should be written with the C programming language, not with pseudocode. Include a brief explanation the purpose of each modiﬁcation. Your explanations should be as concise as possible. Leave the full explanation to the following sections.

**thread.c**

```c
/* add global variable */
int load_avg;

/* add variable in initialization */
void thread_init (void)
{
    ...
    initial_thread->nice = 0;    
    initial_thread->recent_cpu = FP_CONST(0)
    ...    
}
static void
init_thread (struct thread *t, const char *name, int priority)
{	
    ...
    t->nice = 0
    t->recent_cpu = FP_CONST(0)
    ...
}


/* implement empty function */
void thread_set_nice (int nice UNUSED) {}
int thread_get_nice (void) {}
int thread_get_load_avg (void) {}
int thread_get_recent_cpu (void) {}   


/*modify existing function */
void thread_set_priority (int new_priority) {
    ...
} // want to check thread_mlfqs bool 


/* add new functions */
void increase_recent_cpu_by1(void){} // Each time a timer interrupt occurs, recent_cpu is incremented by 1 for running thread, i.e. thread_current(), unless the idle thread is running.
void refresh_load_avg(void){} // using formula, just a global variable update
void refresh_recent_cpu(void){} // update recent cpu for all threads
void update_priority_MLFQS(void) {struct thread *t} 
```

**timer.c**

```c
/*modify existing function */
static void timer_interrupt (struct intr_frame *args UNUSED){} // add time tick check and calls to mlfqs-related funcs 
```



### 2. Algorithms

This is where you tell us how your code will work. Your description should be at a level below the high level description of requirements given in the assignment. We have read the project spec too, so it is unnecessary to repeat or rephrase what is stated here. On the other hand, your description should be at a level above the code itself. Don’t give a line-by-line run-down of what code you plan to write. Instead, you should try to convince us that your design satisﬁes all the requirements, including any uncommon edge cases.

The length of this section depends on the complexity of the task and the complexity of your design. Simple explanations are preferred, but if your explanation is vague or does not provide enough details, you will be penalized. Here are some tips:





### 3. Synchronization

Describe your strategy for preventing race conditions and convince us that it works in all cases. Here are some tips for writing this section:

• This section should be structured as a list of all potential concurrent accesses to shared resources. For each case, you should prove that your synchronization design ensures correct behavior.

• An operating system kernel is a complex, multithreaded program, in which synchronizing multiple threads can be diﬃcult. The best synchronization strategies are simple and easily veriﬁable, which leaves little room for mistakes. If your synchronization strategy is diﬃcult to explain, consider how you could simplify it.

• You should also aim to make your synchronization as eﬃcient as possible, in terms of time and memory.

• Synchronization issues revolve around shared data. A good strategy for reasoning about synchronization is to identify which pieces of data are accessed by multiple independent actors (whether they are threads or interrupt handlers). Then, prove that the shared data always remains consistent.

• Lists are a common cause of synchronization issues. Lists in Pintos are not thread-safe.

• Do not forget to consider memory deallocation as a synchronization issue. If you want to use pointers to struct thread, then you need to prove those threads can’t exit and be deallocated while you’re using them.

• If you create new functions, you should consider whether the function could be called in 2 threads at the same time. If your function access any global or static variables, you need to show that there are no synchronization issues.

• Interrupt handlers cannot acquire locks. If you need to access a synchronized variable from an interrupt handler, consider disabling interrupts.

• Locks do not prevent a thread from being preempted. Threads can be interrupted during a critical section. Locks only guarantee that the critical section is only entered by one thread at a time.



### 4. Rationale

Tell us why your design is better than the alternatives that you considered, or point out any shortcomings it may have. You should think about whether your design is easy to conceptualize, how much coding it will require, the time/space complexity of your algorithms, and how easy/diﬃcult it would be to extend your design to accommodate additional features.



## Additional Questions



1. Consider a fully-functional correct implementation of this project, except for a single bug, which exists in the sema_up() function. According to the project requirements, semaphores (and other synchronization variables) must prefer higher-priority threads over lower-priority threads. However, my implementation chooses the highest-priority thread based on the base priority rather than the eﬀective priority. Essentially, priority donations are not taken into account when the semaphore decides which thread to unblock. Please design a test case that can prove the existence of this bug. Pintos test cases contain regular kernel-level code (variables, function calls, if statements, etc) and can print out text. We can compare the expected output with the actual output. If they do not match, then it proves that the implementation contains a bug. You should provide a description of how the test works, as well as the expected output and the actual output.



2. (This question uses the MLFQS scheduler.) Suppose threads A, B, and C have nice values of 0, 1, and 2 respectively. Each has a recent_cpu value of 0. Fill in the table below showing the scheduling decision and the recent_cpu and priority values for each thread after each given number of timer ticks. We can use R(A) and P(A) to denote the recent_cpu and priority values of thread A, for brevity.



| timer ticks | R(A) | R(B) | R(C) | P(A) | P(B) | P(C) | thread to run |
| ----------- | ---- | ---- | ---- | ---- | ---- | ---- | ------------- |
| 0           |      |      |      |      |      |      |               |
| 4           |      |      |      |      |      |      |               |
| 8           |      |      |      |      |      |      |               |
| 12          |      |      |      |      |      |      |               |
| 16          |      |      |      |      |      |      |               |
| 20          |      |      |      |      |      |      |               |
| 24          |      |      |      |      |      |      |               |
| 28          |      |      |      |      |      |      |               |
| 32          |      |      |      |      |      |      |               |
| 36          |      |      |      |      |      |      |               |

