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
#include <signal.h>
#include <memory.h>
#include <ctype.h>
#include <sys/sem.h>
#include <errno.h>

#define SHM_SIZE 2

typedef union {

    int             val;
    struct semid_ds *buf;
    ushort          *array;
} semun;

/**
 * function name: WriteMessage.
 * The input: message to write.
 * The output: void.
 * The function operation: prints the message to the screen.
*/
void WriteMessage(char *message);

/**
 * function name: GetUserCommand.
 * The input: void.
 * The output: user command.
 * The function operation: Reads command from user.
*/
char GetUserCommand();

int HandleUserCommand(char command, char *data);

int main() {

    key_t         key;
    int           shmid;
    int           semid;
    int           stop;
    int           resultValue;
    char          command;
    char          *data;
    semun         semarg;
    struct sembuf sops[1];

    //Create key.
    key = ftok("318810637.txt", 'A');

    //Check if ftok succeeded.
    if (key < 0) {

        perror("Error: ftok failed.\n");
        exit(1);
    }

    //Get shared memory.
    shmid = shmget(key, SHM_SIZE, 0666);

    //Check if shmget succeeded.
    if (shmid < 0) {

        perror("Error: shmget failed.\n");
        exit(1);
    }

    //Attach to shared memory.
    data = shmat(shmid, NULL, 0);

    //Check if shmat succeeded.
    if (data == (char *) -1) {

        perror("Error: shmat failed.\n");
        exit(1);
    }

    semid = semget(key, 2, 0);

    //Check if semget succeeded.
    if (semid < 0) {

        perror("Error: semget faild.\n");
        exit(1);
    }

    //Set sembuf flag.
    sops->sem_flg = 0;

    stop = 0;

    while (!stop) {

        WriteMessage("Please enter request code\n");

        //Receive command from user.
        command = GetUserCommand();

        //Check if input is legal.
        if(command < 'a' || command > 'i'){

            continue;
        }

        //Lock writer.
        sops->sem_num = 1;
        sops->sem_op  = -1;
        resultValue = semop(semid, sops, 1);

        //Check if semop succeeded.
        if (resultValue < 0) {

            //Check if the semaphore broke.
            if(errno == 43 || errno == 22){

                //TODO delete
                printf("Errno: %d\n", errno);
                break;
            }

            perror("Error: semop failed.\n");
            exit(1);
        }

        //Handle user command.
        stop = HandleUserCommand(command, data);

        printf("Client: Wrote to shared memory\n");

        //Unlock server reader.
        sops->sem_num = 0;
        sops->sem_op  = 1;
        resultValue = semop(semid, sops, 1);

        //Check if semop succeeded.
        if (resultValue < 0) {

            //Check if the semaphore broke.
            if(errno == 43 || errno == 22){

                //TODO delete
                printf("Errno: %d\n", errno);
                break;
            }

            perror("Error: semop failed.\n");
            exit(1);
        }

        printf("Client: unlocked server\n");
    }

    //Detach from shared memory.
    resultValue = shmdt(data);

    //Check if shmdt succeeded.
    if (resultValue < 0) {

        perror("Error: shmdt failed.\n");
        exit(1);
    }

    printf("Exiting client\n");

    //TODO check if there are other resources to clean, like the semaphore
}

void WriteMessage(char *message) {

    int writeResult;

    writeResult = write(1, message, strlen(message));

    //Check if write succeeded.
    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }
}

char GetUserCommand() {

    int  readResult;
    char command[2];

    readResult = read(0, command, 2);

    //Check if read succeeded.
    if (readResult < 0) {

        perror("Error: read failed.\n");
        exit(1);
    }

    command[0] = tolower(command[0]);

    return command[0];
}

int HandleUserCommand(char command, char *data) {

    if (command == 'i') {

        return 1;
    } else {

        //TODO make sure the server finished reading
        data[0] = command;

        return 0;
    }

}