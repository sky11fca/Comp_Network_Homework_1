#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <utmp.h>
#include <sys/wait.h>
#include <sys/socket.h>

#define FIFO_IN ".a_fifo.txt"
#define FIFO_OUT ".a_fifo2.txt"
#define FIFO_EXIT ".a_fifo3.txt"
#define LOGIN_TEMP ".login.bin"
#define USERNAMES "important/Database.txt"
#define BUFFER_SIZE 1024
#define UNKNOWN "UNKNOWN COMMAND"

void exit_handler()
{
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

            if(strcmp(receiver, "exit")==0)
            {
                exit_handler();
            }
            else if(strstr(receiver, "echo")!=0)
            {
                char message[BUFFER_SIZE];
                sscanf(receiver, "%*s %*c %[^\n]", message);
                write(sock[0], message, BUFFER_SIZE);

            }
            else if(strstr(receiver, "login")!=0)
            {
                if(access(LOGIN_TEMP, F_OK)==-1)
                {
                    char user[BUFFER_SIZE];
                    sscanf(receiver, "login : %s", user);
                    FILE * fd=fopen(USERNAMES, "r");
                    char lines[BUFFER_SIZE];

                    while(fgets(lines, BUFFER_SIZE, fd))
                    {
                        lines[strlen(lines)-1]='\0';
                        if(strcmp(lines, user)==0)
                        {
                            creat(LOGIN_TEMP, 0666);
                            int fd_log;
                            if((fd_log=open(LOGIN_TEMP, O_WRONLY))!=-1)
                            {
                                write(fd_log, user, BUFFER_SIZE);
                            }
                            close(fd_log);
                            char msg[BUFFER_SIZE]="Successfully logged as ";
                            strcat(msg, user);
                            write(sock[0], msg, BUFFER_SIZE);
                        }
                    }
                    if(access(LOGIN_TEMP, F_OK)==-1)
                    {
                        write(sock[0], "User not in our database", 25);
                    }
                    fclose(fd);
                }
                else
                {
                    write(sock[0], "Already logged in! Please logout", 33);
                }

            }
            else if(strcmp(receiver, "logout")==0)
            {
                if(access(LOGIN_TEMP, F_OK)!=-1)
                {
                    unlink(LOGIN_TEMP);
                    write(sock[0], "Loging off! You are GUEST!", 27);

                }
                else
                {
                    write(sock[0], "You are ALREADY GUEST", 22);
                }

            }
            else if (strcmp(receiver, "get-logged-user")==0)
            {
                if(access(LOGIN_TEMP, F_OK)!=-1)
                {
                    char userstats[BUFFER_SIZE];
                    struct utmp *ut;
                    setutent();

                    while((ut=getutent())!=NULL)
                    {
                        if(ut->ut_type==USER_PROCESS)
                        {
                            snprintf(userstats, sizeof(userstats), "\n%s -> %s -> %d", ut->ut_user, ut->ut_host, ut->ut_tv.tv_sec);
                            write(sock[0], userstats, BUFFER_SIZE);
                        }
                    }
                    endutent();

                }
                else
                {
                    write(sock[0], "Command forbiden to be executed: You are GUEST", 47);
                }
            }
            else if(strncmp(receiver, "get-proc-info : ", 16)==0)
            {
                if(access(LOGIN_TEMP, F_OK)!=-1)
                {
                    char process_stat[BUFFER_SIZE];
                    int pid;
                    char line_stat[BUFFER_SIZE];
                    char total_info[BUFFER_SIZE];
                    FILE * proc;
                    strcpy(total_info, "\n");
                    sscanf(receiver, "get-proc-info : %d", &pid);
                    snprintf(process_stat, sizeof(process_stat), "/proc/%d/status", pid);

                    proc=fopen(process_stat, "r");
                    if(proc==NULL)
                    {
                        perror("OPEN");
                    }

                    while(fgets(line_stat, BUFFER_SIZE, proc))
                    {
                        if(strncmp(line_stat, "Name:", 5)==0 || strncmp(line_stat, "State:", 6)==0 || strncmp(line_stat, "Pid:", 4)==0 || strncmp(line_stat, "PPid:", 5)==0 || strncmp(line_stat, "VmSize:", 7)==0)
                        {

                            strcat(total_info, line_stat);
                        }
                    }

                    printf("%s\n", total_info);
                    write(sock[0], total_info, BUFFER_SIZE);
                    fclose(proc);
                }
                else
                {
                    write(sock[0], "Command forbiden to be executed: You are GUEST", 47);
                }
            }
            else
            {

                write(sock[0], "UNKNOWN COMMAND!!!", 19);
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

            close(fd);

            write(sock[1], msg, BUFFER_SIZE);

            wait(NULL);

            fd=open(FIFO_OUT, O_WRONLY);
            read(sock[1], outputed, BUFFER_SIZE);
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



