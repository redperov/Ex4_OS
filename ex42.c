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
#include <sys/sem.h>
#include <time.h>
#include <pthread.h>


#define ARR_SIZE 5
#define SHM_SIZE 4096

//Queue node.
typedef struct node {

    char        data;
    struct node *next;
} Node;

//Queue data structure.
typedef struct {

    Node *head;
    Node *rear;
} Queue;

typedef union {

    int             val;
    struct semid_ds *buf;
    ushort          *array;
} semun;

//
//
////int           threadsSemid;
//
//
//
////struct sembuf sbThreads[1];
//////key_t         keyThreads;
//int           semid;
//struct sembuf sops[1];
//semun         semarg;

void Display(Queue *queue);

Queue *InitializeQueue();

Queue *Enqueue(Queue *queue, char data);

Queue *Dequeue(Queue *queue);

char Top(Queue *queue);

int IsEmpty(Queue *queue);

int GenerateRandomValue();

void WriteToFile(pthread_t identifier);

void StartThreadPool(pthread_t *threads);

void *ThreadFunction(void *arg);

int HandleCommand(char command);

int   count_internal;
int   file;
char  *data;
Queue *jobsQueue;
pthread_mutex_t writeMutex;
pthread_mutex_t counterMutex;


int main() {

    //Variable declarations.
    key_t         key;
    int           stop;
    int           shmid;
    int           semid;
    int           resultValue;
    semun         semarg;
    struct sembuf sops[2];
    char  command;

    //Create file.
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
    //Queue *jobsQueue; TODO decide if to leave as global or make local.
    jobsQueue = InitializeQueue();

    //Initialize global counter.
    count_internal = 0;

    semid = semget(key, 1, 0666 | IPC_CREAT);

    //Check if semget succeeded.
    if (semid < 0) {

        perror("Error: semget failed.\n");
        exit(1);
    }

    //Initialize semaphore value to 1.
    semarg.val = 1;

    //Semaphore #1.
    resultValue = semctl(semid, 0, SETVAL, semarg);

    //Check if semctl succeeded.
    if (resultValue < 0) {

        perror("Error: semctl failed.\n");
        exit(1);
    }

    //Semaphore #2.
    resultValue = semctl(semid, 1, SETVAL, semarg);

    //Check if semctl succeeded.
    if (resultValue < 0) {

        perror("Error: semctl failed.\n");
        exit(1);
    }

    //Set sembuf values.
    sops->sem_num = 0;
    sops->sem_flg = 0;

    stop = 0;

    while (!stop) {

        //Lock.
        SemLock(semid, sops);

        //Read command from shared memory.
        command = data[0];

        //Handle command.
        stop = HandleCommand(command);

        //Unlock.
        SemUnlock(semid, sops);
    }

    //TODO free memory.
}

int HandleCommand(char command) {

    //TODO add mutex

    if (command == 'g') {
        //TODO add identifier and internal_count to id.txt file

        return 1;

    } else if (command == 'h') {
        //TODO handle that case

        return 1;
    } else {

        //TODO handle case when I dequeue at the same time?
        Enqueue(jobsQueue, command);
    }

    return 0;
}

void CreateThreadPool(pthread_t *threads) {

    int i;
    int resultValue;

    for (i = 0; i < ARR_SIZE; ++i) {

        resultValue = pthread_create(&threads[i], NULL, ThreadFunction, NULL);

        //Check if pthread_create succeeded.
        if (resultValue < 0) {

            perror("Error: pthread_create failed.\n");
            exit(1);
        }
    }
}

void *ThreadFunction(void *arg) {

    //Variable declarations.
    int stop;
    char command;

    stop = 0;

    while(!stop){

        //TODO lock sem

        //TODO add check somewhere to run only if queue is not empty.
        //Get command from jobs queue.
        command = Top(jobsQueue);
        jobsQueue = Dequeue(jobsQueue);

        if (command == 'f') {

            //Get thread identifier.
            pthread_t identifier = pthread_self();

            //Lock.
            LockMutex(&writeMutex);

            //Write identifier to file.
            WriteToFile(identifier);

            //Unlock.
            UnlockMutex(&writeMutex);

        } else {

            //Perform sleep command.
            SleepCommand(command);
        }
    }

}

void SleepCommand(char command){

    int             x   = GenerateRandomValue();
    int             add = command - '0';
    struct timespec ts  = {0, x};

    //Sleep.
    nanosleep(&ts, NULL);

    //Lock.
    LockMutex(&counterMutex);

    //Add to global counter.
    count_internal += add;

    //Unlock.
    UnlockMutex(&counterMutex);

}

void SemLock(int semid, struct sembuf *sops) {

    int resultValue;

    sops->sem_op = -1;
    resultValue = semop(semid, sops, 1);

    //Check if semop succeeded.
    if (resultValue < 0) {

        perror("Error: semop failed.\n");
        exit(1);
    }
}

void SemUnlock(int semid, struct sembuf *sops) {

    int resultValue;

    sops->sem_op = 1;
    resultValue = semop(semid, sops, 1);

    //Check if semop succeeded.
    if (resultValue < 0) {

        perror("Error: semop failed.\n");
        exit(1);
    }
}

void InitMutexes(){

    int resultValue;

    resultValue = pthread_mutex_init(&writeMutex, NULL);

    //Check if mutex_init succeeded.
    if(resultValue < 0){

        perror("Error: mutex_init failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_init(&counterMutex, NULL);

    //Check if mutex_init succeeded.
    if(resultValue < 0){

        perror("Error: mutex_init failed.\n");
        exit(1);
    }
}

void LockMutex(pthread_mutex_t *mutex){

    int resultValue;

    resultValue = pthread_mutex_lock(mutex);

    //Check if mutex_lock succeeded.
    if(resultValue < 0){

        perror("Error: mutex_lock failed.\n");
        exit(1);
    }
}

void UnlockMutex(pthread_mutex_t *mutex){

    int resultValue;

    resultValue = pthread_mutex_unlock(mutex);

    //Check if mutex_unlock succeeded.
    if(resultValue < 0){

        perror("Error: mutex_unlock failed.\n");
        exit(1);
    }
}

void DestroyMutexes(){

    int resultValue;

    resultValue = pthread_mutex_destroy(&writeMutex);

    //Check if mutex_destroy succeeded.
    if(resultValue < 0){

        perror("Error: mutex_destroy failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_destroy(&counterMutex);

    //Check if mutex_destroy succeeded.
    if(resultValue < 0){

        perror("Error: mutex_destroy failed.\n");
        exit(1);
    }
}

int GenerateRandomValue() {

    int randomVal;

    srand(time(NULL));

    //Generate random value.
    randomVal = (rand() % 90) + 10;

    return randomVal;
}

void WriteToFile(pthread_t identifier) {

    int  writeResult;
    char buff[256];

    sprintf(buff, "thread identifier is %d and internal_count is %d\n",
            identifier, count_internal);

    writeResult = write(file, buff, strlen(buff));

    //Check if write succeeded.
    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
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

//TODO delete if not needed
void Display(Queue *queue) {

    Node *temp = queue->rear;

    while (temp != NULL) {

        printf("%c -> ", temp->data);
        temp = temp->next;
    }

    printf("NULL");
}