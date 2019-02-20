Design Document for Project 1: Threads
======================================

## Group Members

* Yifan Ning <yifanning@berkeley.edu>
* Zhi Chen <zhichen98@berkeley.edu>
* Jerry Li <jerry.li.cr@berkeley.edu>
* Bart Ba <bart98@berkeley.edu>



## Task 1: Eﬃcient Alarm Clock

### 1. Data structures and functions

Write down any struct deﬁnitions, global (or static) variables, typedefs, or enumerations that you will be adding or modifying (if it already exists). These deﬁnitions should be written with the C programming language, not with pseudocode. Include a brief explanation the purpose of each modiﬁcation. Your explanations should be as concise as possible. Leave the full explanation to the following sections.



###2. Algorithms

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



## Task 2: Priority Scheduler



### 1. Data structures and functions

Write down any struct deﬁnitions, global (or static) variables, typedefs, or enumerations that you will be adding or modifying (if it already exists). These deﬁnitions should be written with the C programming language, not with pseudocode. Include a brief explanation the purpose of each modiﬁcation. Your explanations should be as concise as possible. Leave the full explanation to the following sections.



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

