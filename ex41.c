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

#define SHM_SIZE 4096

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

int main(){

    key_t key;
    int shmid;
    int stop;
    char command;
    char *data;

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

    stop = 0;

    while(!stop){

        WriteMessage("Please enter request code\n");

        command = GetUserCommand();
        stop = HandleUserCommand(command, data);
    }


}

void WriteMessage(char *message){

    int writeResult;

    writeResult = write(1, message, strlen(message));

    //Check if write succeeded.
    if (writeResult < 0) {

        perror("Error: write failed.\n");
        exit(1);
    }
}

char GetUserCommand(){

    int readResult;
    char command;

    readResult = read(0, &command, 1);

    //Check if read succeeded.
    if (readResult < 0) {

        perror("Error: read failed.\n");
        exit(1);
    }

    command = tolower(command);

    return command;
}

int HandleUserCommand(char command, char *data){

    if(command == 'i'){

        //TODO clean memory and exit
        return 1;
    }
    else{

        //TODO make sure the server finished reading
        data[0] = command;

        return 0;
    }

}