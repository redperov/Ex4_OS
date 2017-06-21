/******************************************
* Student name: TODO add
* Student ID: TODO add
* Course Exercise Group: 05
* Exercise name: ex4
******************************************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <time.h>



#define ARR_SIZE 5
#define SHM_SIZE 4096

int count_internal;

typedef struct node {

    char        data;
    struct node *next;
}   Node;

typedef struct {

    Node *head;
    Node *rear;
}   Queue;

void Display(Queue *queue);

Queue *InitializeQueue();

Queue *Enqueue(Queue *queue, char data);

Queue *Dequeue(Queue *queue);

char Top(Queue *queue);

int IsEmpty(Queue *queue);

int GenerateRandomValue();

int main() {

    key_t key;
    int   file;
    int   stop;
    int   shmid;
    char  *data;
    char command;

    file = open("318810637.txt", O_CREAT, 0777);

    //Check if open succeeded.
    if (file < 0) {

        perror("Error: open failed.");
        exit(1);
    }

    //Create key.
    key = ftok("318810637.txt", 'A');

    //Check if ftok succeeded.
    if (key < 0) {

        perror("Error: ftok failed.");
        exit(1);
    }

    //Create shared memory.
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

    //Check if shmget succeeded.
    if (shmid < 0) {

        perror("Error: shmget failed.\n");
        exit(1);
    }

    //Attach to shared memory.
    data = shmat(shmid, NULL, 0);

    //Check if shmat succeeded.
    if (data == (char *) (-1)) {

        perror("Error: shmat failed.\n");
        exit(1);
    }


    //Create threads array.
    pthread_t threads[ARR_SIZE];

    //Create jobs queue.
    Queue *jobsQueue;
    jobsQueue = InitializeQueue();

    //Initialize global counter.
    count_internal = 0;

    stop = 0;

    while(!stop){

        //Read command from shared memory.
        command = data[0];

        stop = HandleCommand(command, threads, jobsQueue, data);
    }

}

int HandleCommand(char command, pthread_t *threads, Queue *jobsQueue, char *data){

    if(command <= 'f'){

        //Add job command to jobs queue.
        Enqueue(jobsQueue, command);
    }
    else if(command == 'g'){

    }
    else{

    }
}

Queue *InitializeQueue() {

    Queue *queue = (Queue *) malloc(sizeof(Queue));
    queue->rear = NULL;
    queue->head = NULL;

    return queue;
}

char Top(Queue *queue) {

    return queue->head->data;
}

int IsEmpty(Queue *queue) {

    return queue->head == NULL;
}

Queue *Enqueue(Queue *queue, char data) {

    Node *newNode = (Node *) malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    if (queue->rear == NULL) {

        queue->rear = newNode;
        queue->head = queue->rear;
    } else {

        Node *temp = queue->rear;
        newNode->next = temp;
        queue->rear   = newNode;
    }

    return queue;
}

Queue *Dequeue(Queue *queue) {

    if (queue->head == NULL) {

        return queue;
    }

    Node *temp = queue->rear;

    while (temp->next != queue->head) {

        temp = temp->next;
    }

    free(queue->head);
    temp->next  = NULL;
    queue->head = temp;

    return queue;
}

void Display(Queue *queue) {

    Node *temp = queue->rear;

    while (temp != NULL) {

        printf("%c -> ", temp->data);
        temp = temp->next;
    }

    printf("NULL");
}

void *ThreadFunction(void *arg){

    char command = *((char*)arg);

    if(command == 'f'){
        //TODO
    }
    else{

        int x = GenerateRandomValue();
        int addToCounter = command - '0';
        struct timespec ts = {0, x };

        //Sleep.
        nanosleep (&ts, NULL);

        //Add to global counter.
        count_internal += addToCounter;

    }
}

int GenerateRandomValue(){

    int randomVal;

    srand(time(NULL));

    //Generate random value.
    randomVal = (rand() % 90) + 10;

    return randomVal;
}

