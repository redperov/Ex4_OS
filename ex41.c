#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>

/******************************************
* Student name: TODO add
* Student ID: TODO add
* Course Exercise Group: 05
* Exercise name: ex4
******************************************/

#define ARR_SIZE 5
#define SHM_SIZE 4096

int count_internal;

int main(){

    int file;
    int key;
    int shmid;
    char *data;


    file = open("318810637.txt", O_CREAT, 0777);

    //Check if open succeeded.
    if(file < 0){

        perror("Error: open failed.");
        exit(1);
    }

    //Create key.
    key = ftok("318810637.txt", 'A');

    //Check if ftok succeeded.
    if(key < 0){

        perror("Error: ftok failed.");
        exit(1);
    }

    //Create shared memory.
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

    //Check if shmget succeeded.
    if(shmid < 0){

        perror("Error: shmget failed.\n");
        exit(1);
    }

    //Attach to shared memory.
    data = shmat(shmid, NULL, 0);

    //Check if shmat succeeded.
    if(data == (char*)(-1)){

        perror("Error: shmat failed.\n");
        exit(1);
    }


    //Create threads array.
    pthread_t threads[ARR_SIZE];
    //TODO create job queue
    count_internal = 0;
}

