#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_IN "a_fifo.txt"
#define FIFO_OUT "a_fifo2.txt"
#define FIFO_EXIT "a_fifo3.txt"
#define LOGIN_TEMP "login.bin"
#define BUFFER_SIZE 1024
#define PS "~>"

int main()
{
    printf("WELLCOME!\nLoged in as: GUEST\n");
    printf("[CLIENT] Enter a message:\n");
    while(1)
    {
        char msg[BUFFER_SIZE];
        char received[BUFFER_SIZE];
        int fd;

        printf("%s ", PS);

        
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strlen(msg)-1]='\0';
        

        fd=open(FIFO_IN, O_WRONLY);

        write(fd, msg, BUFFER_SIZE);

    
        close(fd);

    
        fd=open(FIFO_OUT, O_RDONLY);
        read(fd, received, BUFFER_SIZE);
        close(fd);

        if(access(FIFO_EXIT, F_OK)==0)
        {
            printf("[CLIENT] Now we are exiting the program! goodbye!! :>\n");
            break;
        }
        else
        {
            printf("[CLIENT] -> %s\n", received);
        }

        
        
    }
    unlink(FIFO_IN);
    unlink(FIFO_OUT);
    unlink(FIFO_EXIT);
    unlink(LOGIN_TEMP);

}