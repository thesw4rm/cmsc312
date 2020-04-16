
int num_avail = 3;
int waiting_processes[MAX PROCS];
int num_waiting;
condition c;
void request_printer(int proc_number)
{
    if(num_avail) { // First condition: printers are available
        --num_avail; // Process receives a printer
    }
    else{ // Process needs to wait
        waiting_processes[num_waiting] = proc_number; // Add to list of waiting processes
        ++num_waiting; // Increment index and waiting processes
        sortByPri(waiting_processes); // Sort processes highest priority first
    }
    while(!num_avail && waiting_processes[0] != proc_number){
        // Wait for printer to become available
        wait(c);
        waiting_processes[0] = waiting_processes[num_waiting -1]; // queue next process
        --num_waiting;
        sort(waiting_processes); // Sort processes again
        --num_avail;
    }
}
void release_printer()
{
    ++num_avail;
    signal(c); // Signal that printer available for another process
}
