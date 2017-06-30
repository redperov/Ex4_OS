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

void CreateThreadPool(pthread_t *threads);

void *ThreadFunction(void *arg);

int HandleCommand(char command, pthread_t *threads);

void SleepCommand(char command);

void SemLock(int semid, struct sembuf *sops);

void SemUnlock(int semid, struct sembuf *sops);

void InitMutexes();

void LockMutex(pthread_mutex_t *mutex);

void UnlockMutex(pthread_mutex_t *mutex);

void DestroyMutexes();

void KillAllThreads(pthread_t *threads);

void EndAllThreads(pthread_t *threads);

int             count_internal;
int             file;
int             stopAllThreads;
char            *data;
Queue           *jobsQueue;
pthread_mutex_t writeMutex;
pthread_mutex_t counterMutex;
pthread_mutex_t queueMutex;


int main() {

    //Variable declarations.
    key_t         key;
    int           stop;
    int           shmid;
    int           semid;
    int           resultValue;
    semun         semarg;
    struct sembuf sops[1];
    char          command;

    //Create file.
    file = open("318810637.txt", O_CREAT | O_WRONLY, 0777);

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
    
    //Initialize mutexes.
    InitMutexes();

    //Create thread pool.
    CreateThreadPool(threads);

    //Initialize global counter.
    count_internal = 0;

    semid = semget(key, 2, 0666 | IPC_CREAT);

    //Check if semget succeeded.
    if (semid < 0) {

        perror("Error: semget failed.\n");
        exit(1);
    }

    //Initialize semaphore value to 0.
    semarg.val = 0;

    //Semaphore #0.
    resultValue = semctl(semid, 0, SETVAL, semarg);

    //Check if semctl succeeded.
    if (resultValue < 0) {

        perror("Error: semctl failed.\n");
        exit(1);
    }

    //Initialize semaphore value to 1.
    semarg.val = 1;

    //Semaphore #1.
    resultValue = semctl(semid, 1, SETVAL, semarg);

    //Check if semctl succeeded.
    if (resultValue < 0) {

        perror("Error: semctl failed.\n");
        exit(1);
    }

    //Set sembuf values.
    //sops->sem_num = 0;
    sops->sem_flg = 0;

    stop = 0;

    while (!stop) {

        printf("Waiting for client\n");

        //Wait for client.
        sops->sem_num = 0;
        sops->sem_op  = -1;
        resultValue = semop(semid, sops, 1);

        //Check if semop succeeded.
        if (resultValue < 0) {

            perror("Error: semop failed.\n");
            exit(1);
        }

        //Read command from shared memory.
        command = data[0];

        //Handle command.
        printf("read data\n");
        stop = HandleCommand(command, threads);
        printf("Handled command\n");

        //Unlock client writer.
        sops->sem_num = 1;
        sops->sem_op  = 1;
        resultValue = semop(semid, sops, 1);

        //Check if semop succeeded.
        if (resultValue < 0) {

            perror("Error: semop failed.\n");
            exit(1);
        }

        printf("Released client\n");
    }

    //Delete all semaphores.
    resultValue = semctl(semid, 0, IPC_RMID, semarg);

    //Check if semctl succeeded.
    if (resultValue < 0) {

        perror("Error: semctl delete failed.\n");
        exit(1);
    }

    //Delete all mutexes.
    DestroyMutexes();

    //Detach from shared memory.
    resultValue = shmdt(data);

    //Check if shmdt succeeded.
    if (resultValue < 0) {

        perror("Error: shmdt failed.\n");
        exit(1);
    }

    //Free shared memory.
    resultValue = shmctl(shmid, IPC_RMID, NULL);

    //Check if shmctl succeeded.
    if (resultValue < 0) {

        perror("Error: memory release failed.\n");
        exit(1);
    }

    resultValue = close(file);

    //Check if close succeeded.
    if (resultValue < 0) {

        perror("Error: close failed.\n");
        exit(1);
    }

    //TODO free memory.
}

int HandleCommand(char command, pthread_t *threads) {

    if (command == 'g') {

        //Kill all the threads.
        KillAllThreads(threads);

        return 1;

    } else if (command == 'h') {

        //Lock.
        LockMutex(&queueMutex);

        //Insert job to jobs queue.
        Enqueue(jobsQueue, command);

        //Unlock.
        UnlockMutex(&queueMutex);

        //End all the threads.
        EndAllThreads(threads);

        return 1;
    } else {

        //Lock.
        LockMutex(&queueMutex);

        //Insert job to jobs queue.
        Enqueue(jobsQueue, command);

        //Unlock.
        UnlockMutex(&queueMutex);
    }

    return 0;
}

void KillAllThreads(pthread_t *threads) {

    int resultValue;
    int i;

    printf("Killing all threads\n");

    for (i = 0; i < ARR_SIZE; ++i) {

        resultValue = pthread_cancel(threads[i]);

        //Check if pthread_cancel succeeded.
        if (resultValue != 0) {

            perror("Error: pthread_cancel failed.\n");
            exit(1);
        }

        printf("Killed thread %d\n", i);
    }

    //Get thread identifier.
    pthread_t identifier = pthread_self();

    //Lock.
    LockMutex(&writeMutex);

    //Write identifier to file.
    WriteToFile(identifier);

    //Unlock.
    UnlockMutex(&writeMutex);

}

void EndAllThreads(pthread_t *threads) {

    int resultValue;
    int i;

    //Join all threads.
    for (i = 0; i < ARR_SIZE; ++i) {

        resultValue = pthread_join(threads[i], NULL);

        //Check if pthread_join succeeded.
        if (resultValue != 0) {

            perror("Error: pthread_join failed.\n");
            exit(1);
        }

        printf("joining thread %d\n", i);
    }

    //Write all threads ids.
    for (i = 0; i < ARR_SIZE; ++i) {

        //Get thread identifier.
        pthread_t identifier = threads[i];

        //Lock.
        LockMutex(&writeMutex);

        //Write identifier to file.
        WriteToFile(identifier);

        //Unlock.
        UnlockMutex(&writeMutex);
    }

    //Get thread identifier.
    pthread_t identifier = pthread_self();

    //Lock.
    LockMutex(&writeMutex);

    //Write identifier to file.
    WriteToFile(identifier);

    //Unlock.
    UnlockMutex(&writeMutex);

}

void CreateThreadPool(pthread_t *threads) {

    int i;
    int resultValue;

    stopAllThreads = 0;

    for (i = 0; i < ARR_SIZE; ++i) {

        resultValue = pthread_create(&threads[i], NULL, ThreadFunction, NULL);

        //Check if pthread_create succeeded.
        if (resultValue != 0) {

            perror("Error: pthread_create failed.\n");
            exit(1);
        }
    }
}

void *ThreadFunction(void *arg) {

    //Variable declarations.
    char command;

    while (!stopAllThreads) {

        //Lock.
        LockMutex(&queueMutex);

        //Check if queue is empty.
        if (IsEmpty(jobsQueue)) {

            //Unlock.
            UnlockMutex(&queueMutex);
            continue;
        }

        //Get command from jobs queue.
        command   = Top(jobsQueue);
        jobsQueue = Dequeue(jobsQueue);

        printf("Took command %c\n", command);

        //Check if need to stop threads.
        if (command == 'h') {

            stopAllThreads = 1;
        }

        //Unlock.
        UnlockMutex(&queueMutex);

        //Check if threads need to stop.
        if (stopAllThreads) {

            break;
        }

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

            printf("Entering sleep command.\n");
            //Perform sleep command.
            SleepCommand(command);
        }
    }

    printf("Exiting thread\n");

    //TODO add pthread_exit if needed.
}

void SleepCommand(char command) {

    int             x   = GenerateRandomValue();
    int             add = command - 'a' + 1;
    struct timespec ts  = {0, x};
    int             resultValue;

    printf("Starting sleep.\n");

    //Sleep.
    resultValue = nanosleep(&ts, NULL);

    //Check if nanosleep succeeded.
    if (resultValue < 0) {

        perror("Error: nanosleep failed.\n");
        exit(1);
    }

    printf("Finished sleep.\n");

    //Lock.
    LockMutex(&counterMutex);

    //Add to global counter.
    count_internal += add;

    printf("Counter: %d\n", count_internal);

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

void InitMutexes() {

    int resultValue;

    resultValue = pthread_mutex_init(&writeMutex, NULL);

    //Check if mutex_init succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_init failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_init(&counterMutex, NULL);

    //Check if mutex_init succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_init failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_init(&queueMutex, NULL);

    //Check if mutex_init succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_init failed.\n");
        exit(1);
    }
}

void LockMutex(pthread_mutex_t *mutex) {

    int resultValue;

    resultValue = pthread_mutex_lock(mutex);

    //Check if mutex_lock succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_lock failed.\n");
        exit(1);
    }
}

void UnlockMutex(pthread_mutex_t *mutex) {

    int resultValue;

    resultValue = pthread_mutex_unlock(mutex);

    //Check if mutex_unlock succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_unlock failed.\n");
        exit(1);
    }
}

void DestroyMutexes() {

    int resultValue;

    resultValue = pthread_mutex_destroy(&writeMutex);

    //Check if mutex_destroy succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_destroy failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_destroy(&counterMutex);

    //Check if mutex_destroy succeeded.
    if (resultValue < 0) {

        perror("Error: mutex_destroy failed.\n");
        exit(1);
    }

    resultValue = pthread_mutex_destroy(&queueMutex);

    //Check if mutex_destroy succeeded.
    if (resultValue < 0) {

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

    unsigned long id = (unsigned long) identifier;

    printf("Writing to file id %lu\n", id);

    sprintf(buff, "thread identifier is %lu and internal_count is %d\n",
            id, count_internal);

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


    if (queue->head == NULL) {

        queue->head = newNode;
    } else {

        queue->rear->next = newNode;
    }

    queue->rear = newNode;

    return queue;
}

Queue *Dequeue(Queue *queue) {

    Node *save;

    if (queue->head == NULL) {

        return queue;
    } else {

        save = queue->head;
        queue->head = queue->head->next;
        free(save);
    }

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