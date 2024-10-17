#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define FIFO_IN "a_fifo.txt"
#define FIFO_OUT "a_fifo2.txt"
#define FIFO_EXIT "a_fifo3.txt"
#define BUFFER_SIZE 1024

void exit_handler()
{
    //printf("FUNC CALLED\n");
    mkfifo(FIFO_EXIT, 0666);
}

int main()
{
    while(1)
    {
    mkfifo(FIFO_IN, 0666);
    mkfifo(FIFO_OUT, 0666);

    pid_t pid;
    int sock[2];

    int fd, fd_exit;
    fd=open(FIFO_IN, O_RDONLY);

    socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
    pid=fork();
    if(pid==0)
    {
        close(sock[1]);
        char receiver[BUFFER_SIZE]="\0";
        read(sock[0], receiver, BUFFER_SIZE);

        //TODO ADD THE SAID INSTRUCTIONS:
        //LOGIN:USER
        //GET-LOGGED-USER
        //GET-PROC-INFO:PID
        //LOGOUT
        //printf("%s\n", receiver);

        if(strcmp(receiver, "exit")==0)
        {
            exit_handler();
        }
        else
        {
        //printf("SUCCESFULLY received message from father!!\n");
        //printf("OUTPUTED MESSAGE: %s\n", receiver);
        if(write(sock[0], receiver, BUFFER_SIZE)<0)
        {
            perror("SERVER C: WRITE!!");
            exit(1);
        }
        }
        close(sock[0]);
        exit(1);

    }
    else
    {
        close(sock[0]);
        char msg[BUFFER_SIZE];
        char outputed[BUFFER_SIZE];
        

        read(fd, msg, BUFFER_SIZE);
        //printf("OUTPUTED MESSAGE: %s\n", msg);
        close(fd);
        // if(strcmp(msg, "exit")==0)
        // {
        //     break;
        // }
        // //printf("Sending the Message to the child...\n");
        write(sock[1], msg, BUFFER_SIZE);

        wait(NULL);

        fd=open(FIFO_OUT, O_WRONLY);
        read(sock[1], outputed, BUFFER_SIZE);
        //printf("%s\n", outputed);
        write(fd, outputed, BUFFER_SIZE);
        close(sock[1]);
        close(fd);
        if(access(FIFO_EXIT, F_OK)==0)
        {
            break;
        }
        
    }
    }

} 



   // char msg[BUFFER_SIZE];
    // int fd;

    // fd=open(FIFO_IN, O_RDONLY);

    // read(fd, msg, BUFFER_SIZE);

    // printf("SERVER: Received this lovely msg: %s\n", msg);
    // close(fd);
    // msg[0]='P';
    // fd=open(FIFO_OUT, O_WRONLY);

    // write(fd, msg, BUFFER_SIZE);
    // close(fd);
