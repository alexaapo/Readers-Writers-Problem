# Readers-Writers-Problem
I create a programm that produces processes in order to create this architecture of interprocessing communication:

![Screenshot from 2020-09-17 12-40-05](https://user-images.githubusercontent.com/60033683/93454350-891f0e80-f8e3-11ea-8cf9-3fc8b0090a39.png)

The original program is called a coordinator. Peer processes are activated for a certain number of iterations. In each iteration, they attempt to gain access to one of the memory entries. The number of entries is given as a parameter to coordinator program and guides the shared memory allocation. A possible operation of a peer can perform in the entry is either read or write. In the case of reading, multiple readers can use the same entry at the same time. In case of writing, we exclude any other competitors (peers). The behavior of the process (read or write) occurs randomly. The readers / writers ratio is an argument to the coordinator (from the command line). The choice of the entry in which the process will take place is also random (uniformly within the number of entries). 

The commitment of an entry from a process lasts exponentially distributed random time. The process prints the average waiting time for the attachment of an entry as well as the number of readings / writings that were attempted. Each time it accesses an entry, the number of readings / writings will be increased.

When all iterations of each process are over, I print the statistics (average waiting time for commitment, number of readings, number of writings). After the termination of peers, the coordinator will scan the shared memory by reading the entered values and presenting the sums (readings / writings). Then the shared memory is released as well as the synchronization structures. 

**You can perform multiple executions of the program for different numbers of entries, processes (peers) as well as percentage of readers / writers to observe and compare the results with each other.**

## Execution 
1) **make**

2) **./coordinator number_of_peers number_of_Entries Ratio Iteration Times**

3) **make clean**
