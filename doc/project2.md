Design Document for Project 1: Threads
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

## Task 1: Eﬃcient Alarm Clock

<br />

### 1. Data Structures and functions

**timer.c**

```c
/* Modification based on the original code */
void timer_sleep (int64_t ts){
  ...
  current_thread->wake_tick = ts + ticks;                           /* Set wake_tick */
  thread_block();                                                   /* Block current thread */
  ...
}

/* Modify timer_interrupt to make operations atomic, evoke unblock_check using thread_sleep_foreach */
static void timer_interrupt (struct intr_frame, *args UNUSED){...}  
```

**thread.h**

```c
/* Addition */
void unblock_check (struct thread *t, void *aux);        /* Func for thread_sleep_foreach */
void thread_sleep_foreach (thread_action_func *, void *aux);       /* Apply func for all threads in sleep_list */


/* Modification */
struct thread {
	...
	int64_t wake_tick;                   /* Amount of time before wake */
	struct list_elem sleep_elem;         /* List element for the sleeping list. */
	...
}
```


**thread.c**

```c
/* Addition */

/* List of sleeping threads */
static struct list sleep_list;

/* Apply func for all threads in sleep_list */
void thread_sleep_foreach (thread_action_func *, void *aux){...}

/* Comparator to sort sleep_list depending on wakeTick, uses list_insert_ordered */
bool thread_sleeper_more(const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...}

/* Func for thread_sleep_foreach, can call thread_unblock */
void unblock_check (struct thread *t, void *aux){...};


/* Modification */

static void init_thread (struct thread *t, const char *name, int priority){
	...
	t->wake_tick = -1;    /* Initialize wake_tick of each thread */
	...
}

/* Initialize current_tick and sleep_list */
thread_init (void){
  ...
  list_init (&sleep_list);
  ...
}

/* Insert thread into sleep_list with thread_sleeper_more, because we want to put small elements in the front */
thread_block (void){...}

/* Pop thread from sleep_list */
thread_unblock (void){...}
```

<br />

### 2. Algorithms
 - `time_sleep()`: Calculate the value of (current tick + sleep tick) and save it as `wait_tick` of the current thread. It represents when the current thread will wake up. Call `thread_block()` to put itself to sleep, which changes the thread status to `THREAD_BLOCKED` and then place it to `sleep_list`.
 
 - `timer_interrupt()`: This function increments the current time tick. Then, it calls `thread_sleep_foreach`
 
 - `thread_sleep_foreach()`: It go through all threads in `sleep_list`. For each thread from the beginning, it wakes and pops it if its `wake_tick` is earlier than or the same as current time. In this case, we call `thread_unblock()` on the corresponding thread so it goes to `ready_list` and changes status to `THREAD_READY`. We keep doing this until the beginning thread of `sleep_list` has a `wake_tick` larger than the current tick or `sleep_list` is empty.

<br />

### 3. Synchronization
 - This design makes sure that there's always only one thread being added to the `sleep_list` even when multiple threads call this function at the same time. Such synchronization is made sure by sharing a `sleep_list` across threads.

<br />

### 4. Rationale
 - We have considered storing `sleep_ticks` in each thread instead of `wake_tick`. However, in that case, we have to change `sleep_ticks` for all threads in `sleep_list` every time, which is not as efficient.

 - To minimize the amount of time spent in interrupt, this design keeps a sleep list that is sorted based on the time for it to wake up. Such an approach is going to make the interrupt handler run as fast as possible. 
 
 - `sleep_list` becomes a part of a critical section. This makes it atomic and makes sure that there's always only one thread being added at a time. 
 
 - `timer_interrupt()` is also supposed to be atomic, but we don't have to put it into a critical section because it runs on CPU.
 
 - Assert the input tick value to make sure it falls within a plausible bound.
 
 - To make sure these executions run atomically and be consistent with OS behaviors, we disable interrupt before entering the following: `thread_unblock()`, `sleep_list_insert()`, `sleep_list_remove()`.


<br />
<br />
<br />
<br />


## Task 2: Priority Scheduler


<br />

### 1. Data structures and functions

We redefine the original ``` priority ``` variable in thread as ``` priority_effective ```.

**In thread.h**

 - Add a lock pointer ``` waiting_lock ``` which keeps track of a lock which is acquired by the thread. <br />
 - Add a list ``` locks ``` which stores all the needed locks. <br />
 - Add an integer ``` priority_ori ``` which stores the original priority of the thread. This should not be influenced by donation and it can only be changed when init and set priority.

 - Add a function ``` thread_get_lock ``` which takes in a lock pointer.<br />
 - Add a function ``` thread_rm_lock ``` which takes in a lock pointer.<br />
 - Add a function ``` thread_donate_priority ``` which takes in a thread pointer.<br />
 - Add a function ``` thread_update_priority ``` which takes in a thread pointer.<br />
 - Add a function ``` thread_priority_less ``` which is fed into ``` list_insert_ordered ``` function of list. <br />

```c
/* Add new variables */
struct thread
{
  struct lock *waiting_lock;          /* A lock acuired by the thread */
  struct list locks;                  /* Stores all the needed locks */
  int pritority_ori;                  /* Stores the original priority of the thread */
  ...
}

/* Add new functions */
void thread_get_lock (struct lock *);
void thread_rm_lock (struct lock *);
void thread_donate_priority (struct thread *);
void thread_update_priority (struct thread *);
bool priority_less(const struct list_elem *e1, const struct list_elem *e2, void *aux);
```


**In sync.h**

 - in ``` struct lock ```, we add two variables:

``` struct list_elem elem ```, which is used to store lock in a linked list.<br />
``` int priority_max ```, which keeps track of the highest priority of all the threads that acquire the current lock. <br />

 - We also have to add two functions which are served as arguments of ``` list_insert_ordered ``` function:

``` lock_priority_less ``` and ``` cond_sema_priority_less ```, both of which take in two ``` list_elem ``` and ``` aux ```.

```c
/* Add two variables in struct lock */
struct lock
  {
    struct list_elem elem;          /* Elem to store in list */
    int max_priority;               /* Highest priority of threads which needs this lock */
  }
  
/* Add two less functions */
bool lock_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux);
bool cond_sema_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux);
```



**In thread.c**

 - Change all the ``` list_push_back ``` into ``` list_insert_ordered ``` with ``` thread_priority_less ``` function. There are three functions which involve this amendment: ``` thread_unblock ```, ``` thread_yield ```, ``` init_thread ```.

 - We have to change ``` thread_set_priority ``` function. Here, we have to update the ``` priority_ori ``` as ``` new_pritority ``` and ensure the ``` priority_effective ``` variable of the thread is the max of ``` priority_ori ``` and itself. If ``` priority_effective ``` is changed, we should yield the current thread.

 - In ``` init_thread ```, we have to initialize the new variables added to thread including ``` priority_ori ```, ``` locks ```, ``` waiting_lock ```. 

 - Add a function ``` thread_get_lock ``` which takes in a lock pointer. It inserts a lock into thread's ``` locks ``` list. At the same time, update thread's priority if possible.

 - Add a function ``` thread_rm_lock ``` which takes in a lock pointer. It removes a lock from thread's ``` locks ``` list. At the same time, update thread's priority if possible.

 - Add a function ``` thread_donate_priority ``` which takes in a thread pointer. It should call ``` thread_update_priority ``` to update the thread's priority. After calling ``` thread_update_priority ```, we should consider the case that the thread's priority has been changed and we should update its position in ``` ready_list ```.

 - Add a function ``` thread_update_priority ``` which takes in a thread pointer. We update thread's priority as the max of the largest priority of its ``` locks ``` and its ``` priority_ori ```. We should not consider the old ``` priority_effective ``` value since we might call this function after we remove a lock which might cause the ``` priority_effective ``` to decrease. 

 - Add a function ``` thread_priority_less ``` which is fed into ``` list_insert_ordered ``` function of list.

```c
void thread_get_lock (struct lock *){...}
void thread_rm_lock (struct lock *){...}
void thread_donate_priority (struct thread *){...}
void thread_update_priority (struct thread *){...}
bool priority_less(const struct list_elem *e1, const struct list_elem *e2, void *aux){...}
```


**In sync.c** 

 - In ``` sema_up ```, we have to sort ``` &sema->waiters ``` by threads' ``` priority_effective ```.

 - In ``` lock_acquire ```, we have to handle recursive donation. We designed to store ``` priority_max ``` in lock and then update threads' ``` priority_effective ``` according to ``` priority_max ``` of locks it has. In ``` lock_acquire ```, we should update ``` priority_max ``` according to the ``` priority_effective ``` of the current thread. Use recursion to visit ``` waiting_lock ``` of all layers of threads. After we update all the locks, we sema_down the lock. After getting current_thread, we should clear the ``` lock_waiting ``` variable of this thread and set lock's ``` priority_max ``` as the ``` priority_effective ``` of this thread. Finally, we let the thread get the lock by calling ``` thread_hold_the_lock ```. 

 - In ``` lock_release ```, which has to add a part to remove the lock in the lock list of the corresponding thread by calling ``` thread_remove_lock ```. 

 - In ``` cond_signal ```, we have to sort ``` &cond->waiters ``` given ``` cond_sema_priority_less ``` function.

 - We create two functions ``` lock_priority_less ``` and ``` cond_sema_priority_less ``` which should compare ``` priority_max ``` of two locks and ``` priority_effective ``` of threads corresponding to semaphores or condition variables. 

```c
bool lock_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...}
bool cond_sema_priority_less (const struct list_elem *e1, const struct list_elem *e2, void *aux UNUSED){...}
```



<br />

### 2. Algorithms

<br />

#### Section 1 -- Choosing the next thread to run:

 - To handle the priority issues, we want to change the ``` ready_list ``` into a priority queue keep the threads sorted from threads with high ``` priority_effective ``` to those with low ones. Every time we choose the next thread to run, we need to pop from the front of the ``` ready_list ```. <br />
 
 - Throughout the whole implementation of thread, whenever there is an insert into ``` ready_list ```, we have to make sure that we are using ``` list_insert_sorted ``` with function ``` thread_priority_less ``` as an argument. <br />
 
 - Meanwhile, after we add a thread into ``` ready_list ```, we have to immediately redecide which thread to run by calling ``` thread_yield ```.


#### Section 2 -- Acquiring a lock:

 - In ``` lock_acquire ```, we consider two situations. <br />
The first situation is when ``` lock->holder ``` is ``` NULL ```. In this case, we directly ``` sema_down ``` the lock. <br />
The second situation is when ``` lock->holder ``` is another thread. In this case, we point the ``` waiting_lock ``` of this thread to the lock we have. After that, we have to handle the priority donation recursively. To continue recursing, we need to ensure that the ``` priority_effective ``` variable of the current thread is strictly larger than the ``` priority_max ``` of the lock we have. In each recursion, we update ``` priority_max ``` according to the ``` priority_effective ``` of the current thread and then donate priority to the holder of the lock. Then we access the ``` waiting_lock ``` of the lock's holder. <br />

 - After we update all the locks, we sema_down the lock. After getting current_thread, we should clear the ``` lock_waiting ``` variable of this thread and set lock's ``` priority_max ``` as the ``` priority_effective ``` of this thread. Finally, we let the thread get the lock by calling ``` thread_hold_the_lock ```. 


#### Section 3 -- Releasing a lock:

 - In ``` lock_release ```, we first have to remove the lock in the lock list of the corresponding thread by calling ``` thread_remove_lock ```. Then we set the lock's holder to ``` NULL ```. Finally, we sema up this lock.

 - In addition, we should release the lock when the owner thread exits.


#### Section 4 -- Computing effective priority:

 - In ``` thread_update_priority ```, we set the ``` priority_effective ``` of a given thread to the max value among the ``` priority_ori ``` which represents the originally set priority value of the thread and ``` priority_max ``` of each lock that recorded in the thread's locks. 
 
 - The reason is that the largest ``` priority_max ``` among all locks in ``` locks ``` of a thread means the highest priority of all the threads which needs this lock. 


#### Section 5 -- Priority Scheduling for semaphores and locks:

 - Because lock is implemented by semaphore, we only need to implement priority scheduling of semaphore. 
 
 - To implement this, we just have to sort ``` &sema->waiters ``` in ``` sema_up ``` according to ``` cond_sema_priority_less ``` function, which compares the ``` priority_effective ``` of the corresponding thread of two semas. 


#### Section 6 -- Priority Scheduling for condition variables:

 - Similar to Section 5, we just have to sort ``` &cond->waiters ``` in ``` cond_signal ``` according to ``` cond_sema_priority_less ``` function, which compares the ``` priority_effective ``` of the corresponding thread of two conditional variables. 


#### Section 7 -- Changing thread's priority:

 - In ``` thread_set_priority ``` , we update the ``` priority_ori ``` as ``` new_pritority ``` and ensure the ``` priority_effective ``` variable of the thread is the max of ``` priority_ori ``` and itself. 
 
 - If ``` priority_effective ``` is changed, we should yield the current thread.

<br />

### 3. Synchronization

 - Because we implement priority donation of a thread through the ``` priority_max ``` in its lock, we can always get the maximum priority of all threads which need that lock, no matter in which order the threads acquire the lock, we can always get the right ``` priority_effective ``` of our target thread. 
 
 - For example, we have H, M, L which are three threads with decreasing priority and H acquires a lock in L while M also acquires a lock in L. Whenever we call ``` lock_acquire ``` function, we get the maximum of current thread's ``` priority_effective ``` and lock's ``` priority_max ```. Thus, after both H and M donate priority to L, whatever order they are, we can have L with H's ``` priority_effective ```. 


<br />

### 4. Rationale

 - We considered keeping track of tid of the owners of the lock which each thread wants to acquire. However, we can hardly keep track of the places where use the lock. 
 
 - We also considered applying a max heap data structure to the ready_list. However, we decided to follow the current linked list data structure to decrease the difficulty of implementation while the runtime won't change much. 
 
 - As we designed the donation logic heavily based on the given code and logic, it would be easy to extend our design to accommodate additional features.

<br />
<br />
<br />
<br />


## Task 3: Multi-level Feedback Queue Scheduler (MLFQS)

<br />

### 1. Data structures and functions

**thread.c**

```c
/* Add global variable */
int load_avg;

/* Add variable in initialization */
void thread_init (void)
{
    ...
    initial_thread->nice = 0;    
    initial_thread->recent_cpu = FP_CONST(0);
    ...    
}

static void init_thread (struct thread *t, const char *name, int priority)
{	
    ...
    t->nice = 0;
    t->recent_cpu = FP_CONST(0);
    ...
}


/* Implement empty function, simple grab or update */
void thread_set_nice (int nice UNUSED){...}
int thread_get_nice (void){...}
int thread_get_load_avg (void){...}
int thread_get_recent_cpu (void){...}  


/* Modify functions */

/* Want to check thread_mlfqs bool */
void thread_set_priority (int new_priority){...}

/* Want to check thread_mlfqs flag, if true, ignore priority */
void init_thread (struct thread *t, const char *name, int priority){...}

 /* Add time tick check and calls to mlfqs-related funcs to update load_avg, recent_cpu and priority */
void thread_tick (void){...}


/* Add functions */

/* Each time a timer interrupt occurs, recent_cpu is incremented by 1 for running thread, i.e. thread_current(), unless the idle thread is running */
void increase_recent_cpu_by1(void){...}

/* Using formula, just a global variable update */
void refresh_load_avg(void){...}

/* Update recent CPU for a thread */
void refresh_recent_cpu(struct thread *t){...}

/* Update priority for a thread */
void refresh_priority_MLFQS(struct thread *t){...}   
```



**thread.h**

```c
#define NICE_MAX 20
#define NICE_MIN -20
#define NICE_DEFAUT 0

/* Add variables */
struct thread { ...
    int nice;                       
    int recent_cpu;             
    ...
}

/* Add functions */

/* Each time a timer interrupt occurs, recent_cpu is incremented by 1 for running thread, i.e. thread_current(), unless the idle thread is running */
void increase_recent_cpu_by1(void);

/* Using formula, just a global variable update */
void refresh_load_avg(void);

/* Update recent CPU for a thread */
void refresh_recent_cpu(struct thread *t);

/* Update priority for a thread */
void refresh_priority_MLFQS(struct thread *t);    
```


<br />

### 2. Algorithms



- We use the boolean variable `thread_mlfqs` to check whether we should use MLFQS related functions to update priority, and also prevent `thread_set_priority`, and no priority donation is done. If true, we also ignore the `priority` argument in `init_thread()`.

- In `thread_tick()`, we first check whether `thread_mlfqs` flag is true. Since we are going to synchronize thread states,  we disable interrupts and call `increase_recent_cpu_by1`. Then we start check time ticks and iterate through all the thread elements in the `all_list`. if `ticks % TIMER_FREQ == 0`, i.e. multiple of a second, we call `refresh_load_avg` and `refresh_recent_cpu` to update `load_avg` and threads' recent CPU. If `ticks % 4 == 0`, we recalculate the priority of all threads.

- To achieve brevity, we here don't use a multi-level of priority queues. Since we are updating the priority of each thread for certain ticks, we can simply use the same `next_thread_to_run` implementation to choose which thread to run next. Similarly, all the other functions should generally work the same.

- Calculate load average and recent CPU: Since these two variables require float point operation, we easily use the functions in `fixed-point.h`. That's also why we initialize `recent_cpu` to be `FP_CONST(0)`.

- Implement `increase_recent_cpu_by1()`: This is called for the current thread, as required by doc, by the function `thread_tick` inside `thread.c` after checking current thread is not idle. 

- Implement `refresh_load_avg()`: This is simply global variable update using some float point operation. `ready_threads` can be easily computed by calling `list_size (&ready_list)`.

- Implement `refresh_recent_cpu()`: To avoid overflow, we first compute _(2 × load_avg)/(2 × load_avg + 1)_, then do the multiplication. 
- Round Robin Implementation: This is simply done by the FIFO queue implementation we have. For example, if A, B, C which are on the the waiting queue have the same highest priority, after A gets removed from the waiting queue, even if it gets enqueued again with the same highest priority, it will be after C, so the queue becomes: B, C, A...


<br />

### 3. Synchronization

 - There is not much synchronization issue for task3 since a thread is no longer able to manipulate priority, and all the priority calculation is done by CPU. The only shared data between threads, load_avg, also can only be changed by CPU.


<br />

### 4. Rationale

- We don't really choose to implement a real MLFQS for certain reasons. First, if we do, we'll need an array of 64 to hold 64 different linked lists for each priority. This is both memory and time inefficient. Second, this creates an inconsistency with our previous implementation, and thus we need to add extra code to do things like array initialization, array iteration, and updates, etc.

- It should be rather easy to extend our code since most of the features in this part is controlled by the one `thread_mlfqs` boolean variable, we also avoid to add extra data structures, so it should be memory efficient.

- As for amount of code, the most part of code should be about implementing `refresh_load_avg(void)`, `refresh_recent_cpu`, `refresh_priority_MLFQS(void)` and modifying `thread_tick()`.

- It makes sense to put the priority update inside `thread_tick()` since it's called directly by the external timer interrupt handler, so all the operations are done for each time tick. 


<br />
<br />
<br />
<br />

## Additional Questions



1. Consider a fully-functional correct implementation of this project, except for a single bug, which exists in the sema_up() function. According to the project requirements, semaphores (and other synchronization variables) must prefer higher-priority threads over lower-priority threads. However, my implementation chooses the highest-priority thread based on the base priority rather than the eﬀective priority. Essentially, priority donations are not taken into account when the semaphore decides which thread to unblock. Please design a test case that can prove the existence of this bug. Pintos test cases contain regular kernel-level code (variables, function calls, if statements, etc) and can print out text. We can compare the expected output with the actual output. If they do not match, then it proves that the implementation contains a bug. You should provide a description of how the test works, as well as the expected output and the actual output.

<br />

*Answer*

The following pseudocode outputs wrongly because the current ``` sema_up ``` unblocks the thread with highest base priority but not effective priority.

```c
Thread A (priority 1){
	acquire Lock_1                              
	create Thread B                                       
	create Thread C
	create Thread D
	print("%s get all needed locks.\n", "A")              /* Breakpoint 1 (BP_1) */
	release Lock_1                                        /* Breakpoint 2 (BP_2) */
}

Thread B (priority 2){
	acquire Lock_2
	acquire Lock_1
	print("%s get all needed locks.\n", "B")
	release Lock_2
	release Lock_1
}

Thread C (priority 3){
	acquire Lock_1
	print("%s get all needed locks.\n", "C")
	release Lock_1
}

Thread D (priority 4){
	acquire Lock_2
	print("%s get all needed locks.\n", "D")
	release Lock_2
}
```

Suppose we have four threads: ``` Thread A ```, ``` Thread B ```, ``` Thread C ``` and ``` Thread D ``` which has priority 1, 2, 3, 4 in order. At the point we get to *BP_1*, D acquires a lock ``` Lock_2 ``` in B and both B, C acquire the same lock ``` Lock_1 ``` in A. We now have the following priority table:

| Thread Name | Base Priority | Effective Priority |
| ----------- | ------------- | ------------------ |
| Thread A    | 1             | 4                  |
| Thread B    | 2             | 4                  |
| Thread C    | 3             | 3                  |
| Thread D    | 4             | 4                  |

 - If we have ``` sema_up ``` with correct logic, we should unblock ``` Thread B ``` when releasing the lock in A at *BP_2* because ``` Thread B ``` has a higher effective priority *(4)* than ``` Thread C ``` *(3)*. Thus, the correct order should be **"ABDC"**.

 - However, if we use the current ``` sema_up ``` which compares base priorities, we actually unlock ``` Thread C ``` because ``` Thread C ``` has a higher base priority *(3)* than ``` Thread B ``` *(2)*. Thus, the current order is **"ACBD"**, which is not correct.

<br />
<br />
<br />
<br />



2. (This question uses the MLFQS scheduler.) Suppose threads A, B, and C have nice values of 0, 1, and 2 respectively. Each has a recent_cpu value of 0. Fill in the table below showing the scheduling decision and the recent_cpu and priority values for each thread after each given number of timer ticks. We can use R(A) and P(A) to denote the recent_cpu and priority values of thread A, for brevity.

<br />

*Answer*

| timer ticks | R(A) | R(B) | R(C) | P(A) | P(B) | P(C) | thread to run |
| ----------- | ---- | ---- | ---- | ---- | ---- | ---- | ------------- |
| 0           | 0    | 0    | 0    | 63   | 61   | 59   | A             |
| 4           | 4    | 0    | 0    | 62   | 61   | 59   | A             |
| 8           | 8    | 0    | 0    | 61   | 61   | 59   | B             |
| 12          | 8    | 4    | 0    | 61   | 60   | 59   | A             |
| 16          | 12   | 4    | 0    | 60   | 60   | 59   | B             |
| 20          | 12   | 8    | 0    | 60   | 59   | 59   | A             |
| 24          | 16   | 8    | 0    | 59   | 59   | 59   | C             |
| 28          | 16   | 8    | 4    | 59   | 59   | 58   | B             |
| 32          | 16   | 12   | 4    | 59   | 58   | 58   | A             |
| 36          | 20   | 12   | 4    | 58   | 58   | 58   | C             |

<br />
<br />
<br />
<br />

3. Did any ambiguities in the scheduler speciﬁcation make values in the table (in the previous question) uncertain? If so, what rule did you use to resolve them?

<br />

*Answer*

It's not certain when multiple threads all have the same highest priority, which thread should be dequeued first. To resolve this problem, "Round Robin" is used, and the threads with the same highest priority are cycled through to kind of ensure "fairness".