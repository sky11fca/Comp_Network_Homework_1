#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_IN "a_fifo.txt"
#define FIFO_OUT "a_fifo2.txt"
#define BUFFER_SIZE 1024

int main()
{
    char msg[BUFFER_SIZE];
    char received[BUFFER_SIZE];
    int fd;

    mkfifo(FIFO_IN, 0666);
    mkfifo(FIFO_OUT, 0666);

    printf("Enter a message:\n");
    fgets(msg, BUFFER_SIZE, stdin);
    msg[strlen(msg)-1]='\0';
    printf("Waiting for the server to respond...\n");
    fd=open(FIFO_IN, O_WRONLY);

    write(fd, msg, BUFFER_SIZE);

    
    close(fd);
    
    fd=open(FIFO_OUT, O_RDONLY);
    read(fd, received, BUFFER_SIZE);
    printf("OUTPUTTED MESSAGE: %s\n", msg);
    close(fd);
    unlink(FIFO_IN);
    unlink(FIFO_OUT);

}