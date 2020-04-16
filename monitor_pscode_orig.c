
int num_avail = 3;
int waiting_processes[MAX PROCS]; //MAX PROC is n
int num_waiting;
condition c; //this is a semaphore

void request_printer(int proc_number)
{
    //two cases: either a printer is open and we can just allocate it or all printers are busy, so we have to wait for a printer to open in order to allocate it
    if (num_avail > 0)
    {
        num_avail--;
        //give this process the printer
        return;
    }
    else //add this process into the array of waiting processes
    {
        waiting_process[num_waiting] = proc_number;
        num_waiting++;
        //sort the list of waiting processes to ensure the highest priority one runs next
        sort(waiting_processes);
    }
    //wait for a printer to open
    while (num_avail == 0 && waiting_processes[0] != proc_number)
    {
        //wait for c to broadcast that a printer is open
        c.wait();
        //queue up the next process to run
        waiting_processes[0] = waiting_process[num_waiting - 1];
        num_waiting--;
        //again make sure that the list of processes is in order
        sort(waiting_processes);
        //give this process a printer
        num_avail--;
    }
}

void release_printer()
{
    num_avail++;
    //signal that a new printer is open using our semaphore
    //broadcast signals all threads, we just need to signal one of them
    c.signal();
}
