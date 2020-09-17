#include "coordinator.h"
#include "semaphores.h"
#include <math.h>

void Initialize_shm1(int entries, Entry** first_entry)
{
    Entry* ptr = *first_entry;
    for(int i=0;i<entries;i++)
    {
        ptr[i].current_reader_counter = 0;
        ptr[i].reader_counter = 0;
        ptr[i].write_counter = 0;
        ptr[i].rw_mutex = sem_create((key_t)(i+1)*234);
        ptr[i].r_mutex = sem_create((key_t)(i+1)*4381);
        printf("Semaphore r %d and rw %d for entry %d are just created\n",ptr[i].r_mutex,ptr[i].rw_mutex,i);
    }
    printf("\n");
}

void Initialize_shm2(int** first_shm2, float ratio, int peers, int iteration_times)
{
    int* ptr2 = *first_shm2;
    ptr2[0] = (ceil(ratio*peers*iteration_times));  //Num of desirable reads from all children.
    ptr2[1] = (peers*iteration_times-ceil(ratio*peers*iteration_times));  //Num of desirable writes from all children.
    ptr2[2] = 0;  //Sum of all reads that children do.
    ptr2[3] = 0;  //Sum of all writes that chilren do.
}

double time_expo(double lambda)
{
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}