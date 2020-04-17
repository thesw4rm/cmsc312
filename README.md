# Run the project
* If doing SIGINT, please wait 1-2 seconds before sending CTRL^C. The program
    stalls for very long time before ending for some reason if the signal is sent too
    fast.
* Extra credit portion is only implemented for FCFS. SJF is more difficult to
    do because you cannot concurrently have user proceses adding jobs to
    printer queue when you also have to potentially update the head every time
    in case of every process being shorter than the one before. 

1) Download or clone the source code
2) Create a folder to build project in (for this example, I'll call it
build) `mkdir build`
3) Navigate into this folder `cd build`
4) Generate CMake configuration `cmake ..`
5) Build the project `make`
6) Run the project `./a3 [user_procs] [printer_procs]`
7) For example, to run with 3 user processes and 2 printer processes,
execute `./a3 3 2`

# Code Explanation

* If a function/variable is not mentioned here, it is not important to the submitted
    version of this project. Most likely, it is for a future version of the
    code that will be used to collect data for the report.
* See `[FILENAME]_[EXT].md` for more information (for example, `main.c` will be
    documented in [MAIN_C.md](MAIN_C.md)

* Documentation is not available for custom semaphores. See in-code comments for those.
    They were taken directly from Assignment 2, with only one change where
    `sem_init` is set to be shared across processes.


# Overall logic


1. Create all printers/users
2. Following is done concurrently and asynchronously
    1. User loops through each slot's mutex and finds available slot
    2. User adds job to slot
3. Following is done one at a time and asynchronously
    1. Printer finds next print job to complete
    2. Printer waits for time proportional to number of bytes in job
    3. Printer prints to console and waits for next job
4. When user completes
    1. Decrement number of user processes left
    2. Inform if all user processes are complete
5. When user completes (printer)
    1. Keep printing until there is nothing left in the queue
    2. Inform if printer queue is empty and user processes are done
    3. Wait to be cancelled. 
6. Main method
    1. Allocate all required memory/variables
    2. Start user and printer functions
    3. Wait for user and printer functions to complete
    4. Cancel all printer threads
    5. Clean up memory
    6. Exit

# If there is anything after this, ignore it

