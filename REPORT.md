# Important Info

PLEASE READ THE [README](README.md) FILE FOR INFORMATION ON HOW TO RUN THE
PROGRAM. If the comments in the code are ever unclear about how the code works,
every file has been documented in its associated `.md` file. So if you need
information on the `user()` function of `main.c`, the information will be
available in `MAIN_C.md`. More info on how to see this is also documented in
[README](README.md).


# Outline

## Working parts
        * Only FCFS is implemented for extra credit, SJF portion is in another
        folder. However, the FCFS and SJF in the report are based on timings
        from the non-extra credit version of both algorithms. 
        * User processes create jobs using array of semaphores which act as
            mutex for each slot in printer queue
        * Printer processes print jobs in sequence
        * Signal interrupt on the program is implemented, and cleans up
            resources properly. 
        * Everything in the directions including extra credit has been
            implemented for FCFS

## Program Logic

### Terminating condition

The main method uses semaphores to wait for user and printer processes to be
complete. It firsts waits for the user processes to be complete in
`uprocs_complete` and then for printer processes to be complete in `pprocs_complete`.

The user processes automatically terminate and decrement number of user
processes remaining once they have completed their number of jobs. Printer
threads wait for the printer queue to be empty (`head == NULL`) and for user
processes left to be 0, and then they also tell the main method that printing
is complete. i

### Sharing book-keeping variables

All semaphores and other book-keeping variables were stored in shared memory
allocated via mmap. 

### Signal Handler

Whem main method receives sigint, it triggers the main method's SIGINT handler.
This sends a SIGINT to all the user processes. Every user process will flip a
global variable called `sigint_tripped` (because they are forks, they each
have their own copy of the variable). They will then add their next job and
then terminate. 

The printer threads will resume execution as normal, clearing out the remaining
jobs in the printer queue and then informing main method they are ready for
termination because no user processes are left to run. 

The main method clears out remaining allocated memory as normal. 

### FCFS and SJF difference

FCFS and SJF were implemented in separate folders because FCFS is extra credit.
However, one key difference remains where FCFS does not need to update head
every time it adds a job to the end of the LinkedList, unless head was NULL
initially. However SJF adds a job via insertion sort with respect to fewest
number of bytes, so if a new job is shorter than the head, the head has to be
replaced with this new job. The `head` is what I user as the `buffer_index`.

## Sample run output

### FCFS - 1 user process, 6 printer threads

`./a3 1 6`

```text
User 0 is creating 11 jobs
Printer 0 started 1
Printer 2 started 1
Printer 5 started 1
Printer 1 started 1
Printer 3 started 1
Printer 4 started 1
User 0 created job of 238 bytes
User 0 created job of 302 bytes
Printer 0 printed job of 238 bytes created by user 0
User 0 created job of 390 bytes
User 0 created job of 447 bytes
Printer 2 printed job of 302 bytes created by user 0
User 0 created job of 113 bytes
User 0 created job of 791 bytes
User 0 created job of 620 bytes
Printer 3 printed job of 113 bytes created by user 0
User 0 created job of 192 bytes
User 0 created job of 391 bytes
User 0 created job of 696 bytes
Printer 5 printed job of 390 bytes created by user 0
User 0 created job of 337 bytes
USER PROC 0 IS FINISHED
USER PROCS DONE
Printer 2 printed job of 192 bytes created by user 0
Printer 1 printed job of 447 bytes created by user 0
Printer 3 printed job of 391 bytes created by user 0
Printer 2 printed job of 337 bytes created by user 0
Printer 0 printed job of 620 bytes created by user 0
Printer 4 printed job of 791 bytes created by user 0
Printer 5 printed job of 696 bytes created by user 0
7.477286
```


## Graphs/Data Plot

The data was plotted for FCFS vs SJF only in the `a3` folder. This was to
ensure that there are no additional factors added to data collection by the
different user function algorithm implemented in this program. 
