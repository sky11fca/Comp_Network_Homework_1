#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FIFO_IN ".a_fifo.txt"
#define FIFO_OUT ".a_fifo2.txt"
#define FIFO_EXIT ".a_fifo3.txt"
#define LOGIN_TEMP ".login.bin"
#define BUFFER_SIZE 1024
#define PS "~>"

int main()
{
    printf("Waiting for the server to initialize\n");
    while(access(FIFO_IN, F_OK)==-1 && access(FIFO_OUT, F_OK)==-1){}
    printf("WELLCOME!\nLoged in as: GUEST\n");
    printf("[CLIENT] Enter a message:\n");
    while(1)
    {
        char msg[BUFFER_SIZE];
        char received[BUFFER_SIZE];
        int msg_len;
        int received_len;
        int fd;

        printf("%s ", PS);

        
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strlen(msg)-1]='\0';
        msg_len=strlen(msg);
        

        fd=open(FIFO_IN, O_WRONLY);

        write(fd, &msg_len, sizeof(msg_len));
        write(fd, msg, msg_len+BUFFER_SIZE);

    
        close(fd);

    
        fd=open(FIFO_OUT, O_RDONLY);
        read(fd, &received_len, sizeof(received_len));
        read(fd, received, BUFFER_SIZE);
        close(fd);
        printf("[SERVER] -> %s\n", received);
        if(access(FIFO_EXIT, F_OK)==0 || strcmp(received, "exit")==0)
        {
            break;
        }
        
        
    }
    unlink(FIFO_IN);
    unlink(FIFO_OUT);
    unlink(FIFO_EXIT);
    unlink(LOGIN_TEMP);

}