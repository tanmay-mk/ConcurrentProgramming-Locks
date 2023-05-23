# Lab 2 Write-up
This writeup contains    
* A description of algorithms & challenges overcome   
* A brief discussion of performance
* A brief description of code organization   
* Description of every file submitted
* Extant bugs, if any  

## A description of algorithms & challenges overcome   
I have implemented total of 7 algorithms, namely: `tas lock`, `ttas lock`, `ticket lock`, `mcs lock`, `peterson lock` with sequential consistency, `peterson lock` with released consistency and `sense reversal barrier`. Apart from these algorithms, I have also used `mutex` lock and `barrier<>` available in the C++ library. 

### Description of Algorithms  
#### 1. TAS Lock
The Test-and-Set (TAS) lock is implemented using Compare-and-Swap (CAS) algorithm, as C++ doesn't have its own TAS implementation. The TAS lock atomically checks if the lock is currently acquired by polling a `flag`. If value of the flag is currently `true`, that means the lock is currently acquired and a call to CAS returns `false`. In such a way, the thread keeps on polling the flag until and unless the thread itself sets the value of flag to `true`, `atomically`. To achieve this, the TAS lock continuously tries to acquire the lock, by spinning. Once the thread sets the flag to true, it is considered that the thread has acquired the lock. The `TAS` class exports two methods, `TAS::TAS_lock()` which acquires the lock, and `TAS::TAS_unlock()` which releases the lock. While releasing the lock, the thread simply sets the value of flag to `false`, notifying other threads that the lock is now free to be acquired. TAS lock is a `LIFO` lock, which means that the thread which just released the lock has high chances of re-acquiring the lock, which may result into starvation. Thus, TAS lock is an unfair locking scheme.  
#### 2. TTAS Lock
The Test-and-Test-and-Set (TTAS) lock is also implemented using Compare-and-Swap (CAS) algorithm. TTAS lock is very similar to TAS lock. Every attempt to acquire a TAS lock while waiting causes a coherent transition, and chances of cache miss are high. Hence, in TTAS lock, we only attempt to acquire the lock when it is **not** held. The TTAS lock, similar to TAS lock, keeps on polling a flag to check whether the lock is currently held or not. However, the only difference is that instead of constantly trying to acquire the lock, TTAS lock waits patiently till the flag is released by other threads, and only then makes an attempt to acquire the lock. This reduces the chances of cache misses. The `TTAS` class exports two methods, `TTAS::TTAS_lock()` which acquires the lock, and `TTAS::TTAS_unlock()` which releases the lock.Similar to TAS lock, TTAS lock is also a `LIFO` lock, which means that the thread which just released the lock has high chances of re-acquiring the lock, which may result into starvation. Thus, TTAS lock is also an unfair locking scheme.  
#### 3. Ticket Lock
The Ticket Lock is implemented using two variables, `now_serving` & `next_num`. The idea is to create a waiting queue of threads by assigining each thread a 'ticket' number to acquire the lock and access the resource. Every time a thread wishes to acquire the lock, it will be assigned a number `my_num`, and then the thread will keep waiting until `now_serving == my_num`. As soon as the number assigned to thread equals the value of `now_serving`, it is considered as the thread has acquired the lock. To achieve this, we use Fetch-and-Increment (FAI) method, which atomically increments the value of a variable by 1. The `TicketLock` class exports two methods, `TicketLock::Ticket_lock()`, which acquires the lock, and `TicketLock::Ticket_unlock()`, which releases the lock. To release the lock, the thread simply increments the value of `now_serving` by 1 atomically, so that next thread waiting in line will get a chance to acquire the lock. Ticket lock is a `FIFO` lock, which means that if a thread waits long enough, it will eventually get a chance to acqire the lock. Thus, each thread gets equal chance of acquiring the lock, making Ticket lock a fair locking scheme. 
#### 4. MCS Lock
The Mellor-Crummey-Scott (MCS) lock is implemented using a queue for waiting threads. On arrival, we place a node in the queue by appending the node to the queue. The idea is that instead of polling continuously on a global variable (like TAS, TTAS and Ticket locks), we spin on a local node until eventually it's our turn to acquire the lock. The `MCS` class exports two methods, `MCS::acquire()` which acquires the lock, and `MCS::release()` which releases the lock. Upon arrival, we notify our predecessor thread of our presence and desire to acquire the lock after the previous thread is done. While releasing the lock, we check if there is any successor waiting to acquire the lock. If yes, then we notify the next thread that the lock is available, otherwise, we release the lock and queue becomes empty. MCS lock is a `FIFO` lock, which means that if a thread waits long enough, it will eventually get a chance to acqire the lock. Thus, each thread gets equal chance of acquiring the lock, making MCS lock a fair locking scheme. 
#### 5. Peterson's algorithm with Sequential & Released consistency
The Peterson's algorithm for locking is the simplest method of writing a lock using only two threads. If a thread desires to acquire the lock, we notify the system, but first we give other thread a chance to acquire the lock. Then, we wait until the other thread loses the desire to acquire the lock, or it is our turn to acquire the lock. The `Peterson` lock exports 4 methods, `Peterson::sequential_lock()`, which acquires the lock strictly with sequential memory consistency, `Peterson::sequential_unlock()` which releases the lock which was acquired with sequential consistency, `Peterson::released_lock()` which acquires the lock with a mixture of sequential and released memory consistency, and `Peterson::released_unlock()` which releases the lock acquired by `Peterson::released_lock()`. While releasing the lock, we simply notify notify the system that our turn is over, atomically. 
#### 6. Sense Reversal Barrier
Sense Reversal Barrier is a barrier which `flips` its sense every iteration. Barrier is a synchronization method for threads in which threads keep waiting at a barrier untill all threads have arrived, and then all the threads are released together for further execution. The idea is that every time a thread arrives at a barrier, it will flip its own sense, and will keep waiting for all threads to arrive. The last thread to arrive will flip its own sense, along with the global sense of the entire barrier, at which point all threads are notified that the barrier has released the threads. This algorithm is a centralized barrier implementation, which has high contention. The `Barrier` class exports only one method, `Barrier::wait()` which acts as a barrier for all threads. 

### Challenges faced and overcome
The main challenge was to make sure that I don't introduce any latent bugs and memory leaks in my application. Many a times during testing of `bucketsort`, the application was going in a deadlock. It was challenging to debug the deadlock using `gdb`. Implementing the lock itself was bit easy, but incorporating the newly written lock into the existing framework of bucketsort was a bit difficult. For test cases with higher inputs, Jupyter was running out of memory and thus the program was getting killed automatically. There were also several cases of segmentation fault and dangling pointers. Debugging these issues was a great learning experience. Implementing locks for the counter application was very easy. While implementing Peterson's algorithm for released memory consistency, I had to research a lot about how it is used and what are the various ways it can be implemented. Overall, this was a great learning experience. 

## A brief discussion of performance
Based on the data collected by `perf` tool for performance testing of parallel programs, following results were obtained: 
#### For Counter
All algorithms were tested with 4 threads for 1000000 iterations (except Peterson's algorithm, which was tested with 2 threads only).    
| Algorithm              | Runtime (s) | L1-Cache Hit (%) | Branch Prediction Hit (%) | Page Faults | Context Switches |
|------------------------|-------------|------------------|---------------------------|-------------|------------------|
| TAS Lock               | 0.389392    | 87.14            | 99.52                     | 148         | 25               |
| TTAS Lock              | 0.522926    | 98.76            | 99.44                     | 147         | 19               |
| Ticket Lock            | 1.255164    | 99.56            | 99.93                     | 145         | 39               |
| MCS Lock               | 0.602592    | 99.27            | 99.86                     | 147         | 25               |
| Peterson Seq Lock      | 0.293019    | 98.25            | 99.66                     | 139         | 06               |
| Peterson Rel Lock      | 0.599347    | 100              | 100                       | 138         | 10               |
| Pthread Lock           | 0.179125    | 97.25            | 99.27                     | 148         | 5470             |
| Sense Reversal Barrier | 2.688599    | 99.1             | 99.38                     | 151         | 22               |
| Pthread Barrier        | 159.004761  | 92.31            | 96.81                     | 145         | 17797744         |

#### For Bucket sort
All algorithms were tested with 4 threads with a skewed data set of 550000 inputs (except Peterson's algorithm, which was tested with 2 threads only).    
| Lock                   | Barrier                | Runtime (s) | L1-Cache Hit (%) | Branch Prediction Hit (%) | Page Faults | Context Switches |
|------------------------| -----------------------|-------------|------------------|---------------------------|-------------|------------------|
| TAS Lock               | Sense Reversal Barrier | 0.373841    | 98.17            | 99.04                     | 8064        | 10               |
| TTAS Lock              | Sense Reversal Barrier | 0.217075    | 98.92            | 99.07                     | 8070        | 08               |
| Ticket Lock            | Sense Reversal Barrier | 0.286948    | 99.24            | 99.39                     | 8071        | 11               |
| MCS Lock               | Sense Reversal Barrier | 0.386161    | 99.13            | 99.57                     | 8067        | 27               |
| Peterson Seq Lock      | Sense Reversal Barrier | 0.240243    | 98.89            | 99.12                     | 8059        | 03               |
| Peterson Rel Lock      | Sense Reversal Barrier | 0.360756    | 98.96            | 99.34                     | 8058        | 09               |
| Pthread Lock           | Sense Reversal Barrier | 0.57843     | 98.40            | 98.76                     | 8070        | 22466            |
| TAS Lock               | Pthread Barrier        | 0.22065     | 97.75            | 98.00                     | 8067        | 17               |
| TTAS Lock              | Pthread Barrier        | 0.284703    | 98.72            | 99.06                     | 8064        | 24               |
| Ticket Lock            | Pthread Barrier        | 0.293001    | 99.16            | 99.42                     | 8064        | 13               |
| MCS Lock               | Pthread Barrier        | 0.320619    | 99.26            | 99.46                     | 8062        | 29               |
| Peterson Seq Lock      | Pthread Barrier        | 0.24754     | 99.12            | 99.20                     | 8054        | 07               |
| Peterson Rel Lock      | Pthread Barrier        | 0.245768    | 99.03            | 99.16                     | 8060        | 23               |
| Pthread Lock           | Pthread Barrier        | 0.683298    | 96.78            | 97.94                     | 8065        | 37414            |

#### Observations & Analysis
By looking at the data, we can conclude that the cache hit rate for TAS lock is significantly poor than other locking algorithms. This is because the TAS lock continuously tries to acquire the lock even when the lock is currently being acquired, which causes cache misses. While attempting to acquire, the TAS lock continuously tries to write to the memory, but fails. While it attempts to write, all the copies of memory are invalidated and only one modifiable copy of cache line is created. Since the thread fails to write to the memory, the compiler again creates new copies. This keeps happening until the lock is acquired. This can cause a ping-pong reaction between two cores for a single cache line, leading to more cache misses. To tackle this, we use TTAS lock, which has better performance results than TAS lock, which only tries to acquire the lock when the lock is not being held. TAS and TTAS are LIFO locks, which means that the thread which just relesed the lock has high chances of re-acquiring the lock. Therefore, context switches for TAS and TTAS locks are relatively lower than other FIFO locks. Ticket lock is a FIFO lock, hence it has higher context switches than TAS and TTAS lock. Every time a thread relases the ticket lock, a cache miss occurs. By releasing a lock, the thread writes to the memory atomically, thereby invalidating all copies of cache lines. Hence, all other waiting threads undergo a cache miss when lock is released. MCS lock is also a FIFO lock and thus its context switches are higher than other LIFO locks. MCS lock uses a local node to spin, instead of a global node, and thus performs significantly consistent in all scenarios. Peterson lock with sequential consistency behaves poorly in terms of cache hits and branch prediction because compiler takes strict actions to ensure sequential execution of intended program. This impacts the peformance of cache and branch prediction. On the other hand, released consistency is not as strict, and thus it improves cache hits and branch prediction rates. Pthread locks are robust and implemented using various complex algorithms, and evidently it has extremely high number of context switches. Even though it is the fastest one to complete counter application, it is the slowest locking method to complete bucketsort application. Furthermore, cache hit rate and branch prediction rate for pthreads is relatively poor than other locking algorithms. Sense reversal barrier has less context switches and performs much better than pthread lock in both cases. The execution of the program can get really slow when the cores available on the system are limited. For very fair primitives or benchmarks, this can be a huge performance problem. In the counter application's barrier version, for each iteration of the thread, one thread increments the counter and others wait. In order to complete the iteration and continue to the next one, each thread must arrive at the barrier. Now, if we have only 2 cores, threads 1 & 2 will run through the iteration and then have to wait to be descheduled for threads 3&4 to reach the barrier. This keeps on happening because the resources are limited, and the entire execution gets really slow. Similarly, in fair locking schemes such as ticket lock and mcs lock where a thread enters the queue for a lock, then gets descehduled, and their turn arrives while they were descheduled. In this scenario, no other thread can acquire the lock for that time slice. Pthread barrier behaves very poorly in either cases because of exceedingly high number of context switches. The CPU spends most of its time in context switches and has really less time to perform the execution at hand. This impacts the speed, as well as cache hit and branch prediction rate.   

## A brief description of code organization
### Sequential execution of code occurs as follows:
#### For Counter
1. `make` command creates counter executable.     
2. Execute mysort using the following command  
    **A.** `./counter -t <num_threads> -i <num_iterations> -o outputfile.txt --lock=<tas, ttas, mcs, ticket, pthread, petersonseq, petersonrel> --bar=<sense, pthread>`    
    **B.** `./counter --name`    
3. Executing 'counter' strictly requires at least one argument. If user does not provide any argument, the application will exit with status code EXIT_FAILURE.    
4. Once we determine that at least one argument is provided, we start parsing the arguments. The application uses getopt_long() to read the flags starting with either '-' or  '--'.        
5. Flag `--name` does not require any argument. Flag `--lock`, `--bar` , `-o` & `-t` requires arguments. If we do not provide an argument immediately after `--lock` , `--bar` , `-o` or `-t` flag, the application fails.    
6. The application uses a boolean value `nameflag` & `barrierFlag` which is set to true or false based on whether user sets `--name` flag and `--bar` flag during execution of mysort.  
7. When user sets `--name` flag, the application prints the author name, sets the `nameflag` flag to `true` so that sorting doesn't take place, prints the name and exits.   
8. When user sets `-t` flag, the user mentions number of threads required for concurrent execution. This flag is optional, if user does not set this flag, the entire operation is executed by assuming number of threads = 4.   
9. The path of output file is stored in string variable `op_filename`. The user must mention the path of ouptput file using `-o` flag.    
1. Using `-i` flag, the user MUST mention number of iterations that are to be performed by each th thread.       
10. Then, using `--lock` flag, the user specifies which locking algorithm to be used while performing bucketsort. If user does not provide this flag, the counter appication will be performed using default `mutex` lock.   
11. Using `--bar` flag, the user specifies which barrier algorithm is to be used. If the user does not specify this flag, the operation will be executed using default `pthread barrier`. If user mentions this flag, this will set `barrierFlag` to true, and the counter will be incremented using barriers and not locks.   
12. Once parsing of all flags and commands is done, the application checks if the `nameflag` flag is set or not. If it is not set, then it will proceed with counting, otherwise, the application exits after printing author name.
12. If the `nameflag` flag is set to false, the application determines which locking algorithm or barrier to be used for counting based on user inputs, as well as determines the number of threads.   
13. Once all this is done, the application calls `counter()` which returns the final value of counter after the application has been completed.   
14. This returned value is stored to output file.   
15. Then, the application prints the time taken for the application, and exits with return value 0.     

#### For Bucket Sort 
1. `make` command creates mysort executable. 
2. Execute mysort using the following command    
    **A.** `./mysort sourcefile.txt -o outputfile.txt -t <num_threads> --lock=<tas, ttas, mcs, ticket, pthread, petersonseq, petersonrel> --bar=<sense, pthread>`  
    **B.** `./mysort --name`  
3. Executing 'mysort' strictly requires at least one argument. If user does not provide any argument, the application will exit with status code EXIT_FAILURE.   
4. Once we determine that at least one argument is provided, we start parsing the arguments. The application uses getopt_long() to read the flags starting with either '-' or '--'.   
5. Flag `--name` does not require any argument. Flag `--lock`, `--bar` , `-o` & `-t` requires arguments. If we do not provide an argument immediately after `--lock` , `--bar` , `-o` or `-t` flag, the application fails.  
6. The application uses a boolean value `nameflag` which is set to true or false based on whether user sets `--name` flag during execution of mysort.   
7. When user sets `--name` flag, the application prints the author name, sets the `nameflag` flag to `true` so that sorting doesn't take place, prints the name and exits.
8. When user sets `-t` flag, the user mentions number of threads required for concurrent execution. This flag is optional, if user does not set this flag, the entire operation is executed by assuming number of threads = 4.    
9. The path of source file is stored in string variable `ip_filename` and path of output file is stored in string variable `op_filename`. The user must mention the path of ouptput file using `-o` flag.    
10. Then, using `--lock` flag, the user specifies which locking algorithm to be used while performing bucketsort. If user does not provide this flag, the bucket sort will be performed using default `mutex` lock.   
11. Using `--bar` flag, the user specifies which barrier algorithm is to be used. This barrier is only used for synchronization while the master thread records start and end time of the parallel application. If the user does not specify this flag, the operation will be executed using default `pthread barrier`.   
12. Once parsing of all flags and commands is done, the application checks if the `nameflag` flag is set or not. If it is not set, then it will proceed with sorting, otherwise, the application exits after printing author name.   
13. If the `nameflag` flag is set to false, the application declares a vector of int type named as num_list, or `vector<int>num_list;`   
14. After creating a vector, the application calls `readFromFile()` which then opens the source file from the path provided by user, and starts reading integers from the file. As it keeps reading, the application stores each integer in vector num_list. We use an instance of `ifstream` to read the file.    
15. Once all the integers present in the file are read and stored in vector num_list, the application calls `sort_list()` which will sort the entire vector list in ascending order. Locking algorithm, barrier type, number of threads are passed as arguments to `sort_list()`.   
16. In `sort_list()`, we first determine the number of threads to be used for execution. First we check if user has provided any number of threads for execution. If not, we simply select 4 threads. On the other hand, if user has provided number of threads and it is greater than half of list size, we simply reduce the number of threads to half of what user entered. If these two cases are false, and number of threads entered by user is valid, we call the sorting algorithms based on the argument received. Then, we determine which locking & barrier algorithm to be used based on user input. Then we call `bucketsort()` to sort the data set.    
17. Once sorting is done, the application opens the output file based on the path received from user, and starts writing each element in the sorted vector to the file. We use an instance of `ofstream` to write to the file.     
18. Once the file is written, we print the total execution time taken by threads to complete the operation. 
19. After printing time, the code exits with return value = 0.    

## Description of every file submitted
For Lab2, this submission contains a code written in C++ for implementing own locks and barrier algorithms for bucketsort and counter. The code is organized in 2 directories, namely:
**1. bucketsort**    
**2. counter_dir**
Each directory contains 8 files. Description of file in each directory is as follows: 
#### bucketsort
1. **bucketsort.h**   
This is a header file that contains public APIs for bucket sort algorithm. It includes all the library files that are required to perform bucket sort. It also contains declaration of `bucketsort()` which is the API that performs bucketsort on a provided list of integers.  
2. **bucketsort.cpp**   
This file contains the actual source code that performs bucket sort. It also contains numerous private APIs that contribute towards the completion of overall application. `bucketsort()` is a higher level API that calls these private APIs to sort the array. Since these are lower level APIs, they do not need to be exposed to user. They are used internally.    
3. **time.h**   
This is a header file that contains public APIs for timing related opreations. It includes declarations of `getTime()` and `printTimeDifference()` that are used by worker threads in bucket sort and merge sort to record and print time.  
4. **time.cpp**
This file contains actual source code of `getTime()` and `printTimeDifference()`.    
5. **locks.h**  
This is a header file that contains all the base classes required to implement locks. It also contains enumerated lists of all available locking algorithms as well as barrier types. Every member of every class is public since all these members are being used by entire application.      
6. **locks.cpp**
This source file contains all the implementations of locks and barriers. Each implementation is referenced to its base class using scope resolution operator.    
7. **main.cpp**
This file is the application entry point. It contains main(), which parses the command line arguments to decide which algorithm to use for sorting based on user input. It reads inputs from source file, sorts the list of integers and then stores the output in the output file.   
8. **Makefile**
Using a single 'make' command, the compiler will compile all source files and create a single executable named mysort. This mysort executable is then moved to its parent directory. 

#### counter_dir
1. **counter.h**   
This is a header file that contains public APIs for counter application. It includes all the library files that are required to perform counting. It also contains declaration of `counter()` which is the API that performs counting based on a provided number of iterations.  
2. **counter.cpp**   
This file contains the actual source code that performs counting. It also contains numerous private APIs that contribute towards the completion of overall application. `counter()` is a higher level API that calls these private APIs to count. Since these are lower level APIs, they do not need to be exposed to user. They are used internally.    
3. **time.h**   
This is a header file that contains public APIs for timing related opreations. It includes declarations of `getTime()` and `printTimeDifference()` that are used by worker threads in bucket sort and merge sort to record and print time.  
4. **time.cpp**
This file contains actual source code of `getTime()` and `printTimeDifference()`.    
5. **locks.h**  
This is a header file that contains all the base classes required to implement locks. It also contains enumerated lists of all available locking algorithms as well as barrier types. Every member of every class is public since all these members are being used by entire application.      
6. **locks.cpp**
This source file contains all the implementations of locks and barriers. Each implementation is referenced to its base class using scope resolution operator.    
7. **main.cpp**
This file is the application entry point. It contains main(), which parses the command line arguments to decide which algorithm to use for counting based on user input. It calls `counter()`, stores the returned value in a variable, writes the variable to a file. 
8. **Makefile**
Using a single 'make' command, the compiler will compile all source files and create a single executable named counter. This mysort executable is then moved to its parent directory. 
   
      

Outside of these two sub-directories, there are two files: 
1. **Makefile**
Using a single make command, the compiler creates two executables, `counter` for counter application, and `mysort` for sorting application. 
2. **myautograde.sh**  
Leveraging the `autograde.sh` script provided by professor, I created my own autograde script to test the robustness of my algorithms. I am testing each test case with upto 20 threads. As number of threads increase, the time required to execute the application also increases.    

Furthermore, in the directory `my_tests`, I have used the test cases provided in Lab1 for testing of bucketsort, and created few new test cases for counter application.  

## Extant bugs, if any   
At this moment, all 2 tests given in `autograde.sh` are passing without any error. Furthermore, I developed my own few test cases using for counter application and used test cases provided in lab1 for bucketsort testing, and tested them using `autograde.sh` script, and those tests are passing too. In addition to that, I tested all these test cases with upto 20 threads using `myautograde.sh` script, and no errors are seen.  

Hence, at this moment, there are no known bugs.  