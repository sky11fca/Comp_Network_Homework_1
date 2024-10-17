#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_IN "a_fifo.txt"
#define FIFO_OUT "a_fifo2.txt"
#define FIFO_EXIT "a_fifo3.txt"
#define BUFFER_SIZE 1024
#define PS "~>"

int main()
{
    printf("[CLIENT] Enter a message:\n");
    while(1)
    {
        char msg[BUFFER_SIZE];
        char received[BUFFER_SIZE];
        int fd;

        printf("%s ", PS);

        
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strlen(msg)-1]='\0';
        //TEMPORARLY IMPLEMENTATION OF EXIT, PLEASE REMOVE!!!

        

    //printf("OUTPUTED MESSAGE: %s\n", msg);
    //printf("Waiting for the server to respond...\n");
        fd=open(FIFO_IN, O_WRONLY);

        write(fd, msg, BUFFER_SIZE);

    
        close(fd);
        //TEMPORARLY IMPLEMENTATION OF EXIT, PLEASE REMOVE!!!
        //if(strcmp(msg, "exit")==0)
        //{
        //    printf("[CLIENT] Program closing!! GOODBYE!!! :>\n");
        //    break;
        //}
    
        fd=open(FIFO_OUT, O_RDONLY);
        read(fd, received, BUFFER_SIZE);
        close(fd);
        //if(strcmp(received, "exit")==0)
        if(access(FIFO_EXIT, F_OK)==0)
        {
            printf("[CLIENT] Now we are exiting the program! goodbye!! :>\n");
            break;
        }
        else
        {
            printf("[CLIENT] Echoed: %s\n", received);
        }

        
        
    }
    unlink(FIFO_IN);
    unlink(FIFO_OUT);
    unlink(FIFO_EXIT);

}