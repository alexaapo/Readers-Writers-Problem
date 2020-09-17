#pragma once
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//shared memory storages a struct
typedef struct data
{ 
    int current_reader_counter;
    int reader_counter;  //How many times this Entry has been read.
    int write_counter;  
    int rw_mutex;
    int r_mutex;
} Entry;

//Create shared memory and connect
int shmcreate(key_t, int);
//Getting a pointer to the segment
Entry* shmattach(int );
//Detaching segment
int shmdetach(Entry*);
//Deleting shared memory segment
int shmdelete(int);
//Detaching 2nd segment
int shm2detach(int *);
//Getting a pointer to the 2nd segment
int* shm2attach(int);