/* Our first program is a consumer. After the headers the shared memory segment
 (the size of our shared memory structure) is created with a call to shmget,
 with the IPC_CREAT bit specified. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/shm.h>
#include <fcntl.h> 
#include <semaphore.h>


#include "shm_com.h"

#define SHARED_MEM_SIZE 4096


int main()
{
    srand((unsigned int)getpid());    

    void *shared_memory = (void *)0;  
    struct shared_use_st *shared_stuff;
    sem_t * pSem1 = sem_open("toLetClientReadData", O_CREAT, 0666, 0);
    sem_t * pSem2 = sem_open("toLetSenderWriteData", O_CREAT, 0666, 0);
    if(SEM_FAILED == pSem1 || SEM_FAILED == pSem2)
    {
       printf("sem__open error: %s", strerror(errno));
       exit(EXIT_FAILURE);
    }
    printf("========= both semaphores was created.\n");

    int shmid = shmget((key_t)1234, SHARED_MEM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    printf("========= shmget OK.\n");

    /* make the shared memory accessible to the program. */

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    printf("========= shmat OK. Memory attached at %X\n", (int)shared_memory);

/* The next portion of the program assigns the shared_memory segment to shared_stuff,
 which then prints out any text in written_by_you. The loop continues until "end" is found */

    shared_stuff = (struct shared_use_st *)shared_memory;
    int iNext = 0;
    //int nRet;
    printf("Enter a number to start IPC \n");
    int  nCounter = 0;
    scanf("%d", &nCounter);
    int nRet = sem_post(pSem2);
    if(nRet == -1)
    {
        printf("sem__post error: %s \n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    //printf("======== sem__post(pSem2) OK\n");
    while(iNext < 10)
    {
        //printf("----------------Before sem_wait() %d \n", iNext + 1);
        sem_wait(pSem1);
        //printf("----------------after sem_wait() %d \n", iNext + 1);
        printf("Received text: %s", shared_stuff->some_text);       
        //if (strncmp(shared_stuff->some_text, "end", 3) == 0)
        iNext += 1;
        nRet = sem_post(pSem2);
//        if(nRet == -1)
//        {
//            printf("sem__post error: %s \n", strerror(errno));
//            exit(EXIT_FAILURE);
//        }
        //printf("======== sem__post(pSem2) OK  iter= %d\n", iNext);
    }
    sem_close(pSem1);
    sem_close(pSem2);
    sem_unlink("toLetClientReadData");
    sem_unlink("toLetSenderWriteData");
/* Lastly, the shared memory is detached and then deleted. */

    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

