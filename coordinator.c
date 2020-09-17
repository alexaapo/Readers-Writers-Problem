#include <sys/wait.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "shared_memory.h"
#include "semaphores.h"
#include "coordinator.h"

int main (int argc, char* argv[])
{
    srand(time(NULL));
    int peers, entries,pid,iteration_times,status=0;
    float ratio;
    key_t key;

    if(argc !=5){
        printf("Error! You have to insert 4 arguments! \n");
    }
    else{
        peers = atoi(argv[1]);
        entries = atoi(argv[2]);
        ratio = atof(argv[3]);
        iteration_times = atoi(argv[4]);
        if(peers<=0 || entries<=0)
            printf("Error! You have to insert positive numbers! \n");
        if(ratio<0.0 || ratio>1.0)
            printf("Error! Ratio out of bounds! \n");
    }
    //Producing a random key for shared memory.
    if ((key = ftok("coordinator.c", 'R')) == -1) 
    {                
        printf("Error! Cannot create key!   \n");
        exit(1);
    }

    printf("Data from command line:\n");
    printf("Number of peers: %d\n", peers);
    printf("Number of entries: %d\n", entries);
    printf("Ratio number: %f\n", ratio);
    printf("Iterations number: %d\n\n", iteration_times);
    
    //Allocate memory for 1st shared memory.
    int shm_size = entries*sizeof(Entry);
    int shm_id = shmcreate(key, shm_size);
    Entry* first_entry = shmattach(shm_id);

    //Allocate memory for 2nd shared memory.
    int shm2_size = 4*sizeof(int);
    int shm2_id = shmcreate(key, shm2_size);
    int* first_shm2 = shm2attach(shm2_id);

    Initialize_shm1(entries, &first_entry);
    Initialize_shm2(&first_shm2, ratio, peers, iteration_times);
    
    //Create child processes
    for(int i=0; i<peers; i++) 
    {                     
        pid = fork();
        if(pid == -1)
        {
            printf("Fork failed!\n");
            return -1;
        }
        if(pid == 0)  //If its child, do not fork!
            break;
    }

    int read_or_write, reads_of_peer=0, writes_of_peer=0;
    time_t start, end;
    double waiting_average_time;
	srand((unsigned int) getpid()); //Initialize every child with different id.
    //In case, its a child.
    if(pid==0)
    {
        for(int i=0;i<iteration_times;i++)
        {
            read_or_write = rand() % 2;  //0 for reader, 1 for writer.
            int random_num_of_entry = rand() % entries;

            if((read_or_write==0 || first_shm2[1]==0) && first_shm2[0]>0)
            {
                first_shm2[0]--;
                
                start = time(NULL);  //Begin to measure the waiting time to enter in specific Entry.
                if(!(sem_down(first_entry[random_num_of_entry].r_mutex)))  //Reader wants to enter the critical section
                    exit(1);
                end = time(NULL);
                waiting_average_time += ((double)(end - start)); 
                first_entry[random_num_of_entry].current_reader_counter++;
                //There is atleast one reader in the critical section.This ensure no writer can enter if there is even one reader.
                //So we give preference to readers here.
                if(first_entry[random_num_of_entry].current_reader_counter==1)
                {
                    if(!(sem_down(first_entry[random_num_of_entry].rw_mutex)))
                        exit(1);
                }
                //Other readers can enter while this current reader is inside the critical section.
                if(!(sem_up(first_entry[random_num_of_entry].r_mutex)))
                    exit(1);
                
                reads_of_peer++;
                first_entry[random_num_of_entry].reader_counter++;
                //printf("*Reader %d in entry %d,which has %d readers now and %d readers until now!\n\n", getpid(), random_num_of_entry, first_entry[random_num_of_entry].now_reader_counter, shm_ptr[random_num_of_entry].reader_counter);
                sleep(time_expo(0.5));

                if(!(sem_down(first_entry[random_num_of_entry].r_mutex)))  // a reader wants to leave
                    exit(1);
                
                first_entry[random_num_of_entry].current_reader_counter--;
                //If no reader is left in the critical section,
                if(first_entry[random_num_of_entry].current_reader_counter==0)
                {
                    if(!(sem_up(first_entry[random_num_of_entry].rw_mutex)))  //Writers can enter
                        exit(1);
                }    

                if(!(sem_up(first_entry[random_num_of_entry].r_mutex)))  //Reader leaves
                    exit(1);
                
            }
            if((read_or_write==1 || first_shm2[0]==0) && first_shm2[1]>0)
            {
                first_shm2[1]--;
                
                start = time(NULL);
                if(!(sem_down(first_entry[random_num_of_entry].rw_mutex)))      //Writer requests for critical section.
                    exit(1);
                end = time(NULL);
                waiting_average_time+= ((double)(end - start));
                writes_of_peer++;
                first_entry[random_num_of_entry].write_counter++;
                //printf("*Writer %d in entry %d,which has %d writers until now!\n\n", getpid(), random_num_of_entry, first_entry[random_num_of_entry].write_counter);
                sleep(time_expo(0.1));
                if(!(sem_up(first_entry[random_num_of_entry].rw_mutex)))  //Writer leaves.
                    exit(1);
            }    

        }
        first_shm2[2]+=reads_of_peer;  
        first_shm2[3]+=writes_of_peer;
        printf("*Peer:%d, Read times:%d, Write times:%d, Average Waiting Time:%.4f\n",getpid(),reads_of_peer,writes_of_peer,waiting_average_time/iteration_times);
        exit(0);
    }
    else  //In case of father.
    {
        //Wait all children to be done.
        for(int j=0;j<peers;j++)
            wait(&status);
        printf("\n-All peers have read %d times and have writen %d times!\n\n", first_shm2[2],first_shm2[3]);
        
        int reads_of_entries = 0;
        int writers_of_entries = 0;
        
        for(int j=0;j<entries;j++)
        {
            reads_of_entries+=first_entry[j].reader_counter;
            writers_of_entries+=first_entry[j].write_counter;
        }
        printf("-All entries have been read %d times and have been writen %d times!\n\n", reads_of_entries, writers_of_entries);
        for(int j=0;j<entries;j++)
        {            
            printf("*Just deleted a rw-semaphore with id=%d of entry %d\n", first_entry[j].rw_mutex,j);
            sem_del(first_entry[j].rw_mutex);
            printf("*Just deleted a r-semaphore with id=%d of entry %d\n\n", first_entry[j].r_mutex,j);
            sem_del(first_entry[j].r_mutex);
        }
        
        shmdetach(first_entry);
        shmdelete(shm_id);
        printf("*Just detached and deleted the 1st shared memory!\n");  
        shm2detach(first_shm2);
        shmdelete(shm2_id);
        printf("*Just detached and deleted the 2nd shared memory!\n\n"); 
    }
}