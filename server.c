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
#define FILEEXIT ".a_fifo3.txt"
#define LOGIN_TEMP ".login.bin"
#define USERNAMES "important/Database.txt"
#define BUFFER_SIZE 1024
#define UNKNOWN "UNKNOWN COMMAND"

//void exit_handler()
//{
//    mkfifo(FIFO_EXIT, 0666);
//}

int main()
{
    int logged=0;
    int running=1;
    
    while(1)
    {
        mkfifo(FIFO_IN, 0666);
        mkfifo(FIFO_OUT, 0666);

        pid_t pid;
        int sock[2];
        int p1[2], p2[2];
      //  int q1[2], q2[2];

        int fd, fd_exit;
        fd=open(FIFO_IN, O_RDONLY);
        
        pipe(p1);
        pipe(p2);
        
       // pipe(q1);
       // pipe(q2);
        
        socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
        pid=fork();
        if(pid==-1)
        {
            perror("FORK");
            exit(EXIT_FAILURE);
        }
        else if(pid==0)
        {
            close(sock[1]);
            close(p1[1]);
            close(p2[0]);
            char receiver[BUFFER_SIZE]="\0";
            int receiver_len;
            int rec_logged;
            read(sock[0], &receiver_len, sizeof(receiver_len));
            read(sock[0], receiver, BUFFER_SIZE);
            read(p1[0], &rec_logged, sizeof(rec_logged));

            if(strcmp(receiver, "quit")==0)
            {
                char exitmsg[BUFFER_SIZE]="exit";
                int exitmsg_len=5;
                write(sock[0], &exitmsg_len, sizeof(exitmsg_len));
                write(sock[0], exitmsg, BUFFER_SIZE);
            }
            //else if(strstr(receiver, "echo")!=0)
            else if(strncmp(receiver, "echo : ", 7)==0)
            {
                char message[BUFFER_SIZE];
                int message_len;
                sscanf(receiver, "echo : %[^\n]", message);
                message_len=strlen(message);
                write(sock[0], &message_len, sizeof(message_len));
                write(sock[0], message, BUFFER_SIZE);

            }
            //else if(strstr(receiver, "login")!=0)
            else if(strncmp(receiver, "login : ", 8)==0)
            {
                if(rec_logged==0)
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
                            rec_logged=1;
                            int fd_log;
                            if((fd_log=open(LOGIN_TEMP, O_WRONLY))!=-1)
                            {
                                write(fd_log, user, BUFFER_SIZE);
                            }
                            close(fd_log);
                            char msg[BUFFER_SIZE]="Successfully logged as ";
                            int msg_lenght;
                            strcat(msg, user);
                            msg_lenght=strlen(msg);
                            write(sock[0], &msg_lenght, sizeof(msg_lenght));
                            write(sock[0], msg, BUFFER_SIZE);
                            break;
                        }
                    }
                    if(rec_logged==0)
                    {
                        int msg_error1=25;
                        write(sock[0], &msg_error1, sizeof(msg_error1));
                        write(sock[0], "User not in our database", 25);
                    }
                    fclose(fd);
                }
                else
                {
                    int msg_error2=33;
                    write(sock[0], &msg_error2, sizeof(msg_error2));
                    write(sock[0], "Already logged in! Please logout", 33);
                }

            }
            else if(strcmp(receiver, "logout")==0)
            {
                if(rec_logged==1)
                {
                    unlink(LOGIN_TEMP);
                    int msg_logout=27;
                    rec_logged=0;
                    write(sock[0], &msg_logout, sizeof(msg_logout));
                    write(sock[0], "Loging off! You are GUEST!", 27);

                }
                else
                {
                    int msg_logout_err=22;
                    write(sock[0], &msg_logout_err, sizeof(msg_logout_err));
                    write(sock[0], "You are ALREADY GUEST", 22);
                }

            }
            else if (strcmp(receiver, "get-logged-user")==0)
            {
                if(rec_logged==1)
                {
                    char userstats[BUFFER_SIZE];
                    char final_stats[BUFFER_SIZE];
                    strcpy(final_stats, "");
                    int final_stats_len;
                    struct utmp *ut;
                    setutent();

                    while((ut=getutent())!=NULL)
                    {
                        if(ut->ut_type==USER_PROCESS)
                        {
                            snprintf(userstats, sizeof(userstats), "\n%s -> %s -> %d", ut->ut_user, ut->ut_host, ut->ut_tv.tv_sec);
                            strcat(final_stats, userstats);
                        }
                    }
                    final_stats_len=strlen(final_stats);
                    write(sock[0], &final_stats_len, sizeof(final_stats_len));
                    write(sock[0], final_stats, BUFFER_SIZE);
                    endutent();

                }
                else
                {
                    int userstats_len=47;
                    write(sock[0], &userstats_len, sizeof(userstats_len));
                    write(sock[0], "Command forbiden to be executed: You are GUEST", 47);
                }
            }
            else if(strncmp(receiver, "get-proc-info : ", 16)==0)
            {
                if(rec_logged==1)
                {
                    char process_stat[BUFFER_SIZE];
                    int pid;
                    char line_stat[BUFFER_SIZE];
                    char total_info[BUFFER_SIZE];
                    int totalinfo_len;
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

                    totalinfo_len=strlen(total_info);
                    write(sock[0], &totalinfo_len, sizeof(totalinfo_len));
                    write(sock[0], total_info, BUFFER_SIZE);
                    fclose(proc);
                }
                else
                {
                    int procinfo_err=47;
                    write(sock[0], &procinfo_err, sizeof(procinfo_err));
                    write(sock[0], "Command forbiden to be executed: You are GUEST", 47);
                }
            }
            else
            {
                int err_len=19;
                write(sock[0], &err_len, sizeof(err_len));
                write(sock[0], "UNKNOWN COMMAND!!!", 19);
            }
            write(p2[1], &rec_logged, sizeof(rec_logged));
            close(sock[0]);
            close(p1[0]);
            close(p2[1]);
            exit(1);

        }
        else
        {
            close(sock[0]);
            close(p1[0]);
            close(p2[1]);
            char msg[BUFFER_SIZE];
            int msg_len;
            char outputed[BUFFER_SIZE];
            int outputed_len;
        
            read(fd, &msg_len, sizeof(msg_len));
            read(fd, msg, BUFFER_SIZE);

            close(fd);
    
            write(sock[1], &msg_len, sizeof(msg_len));
            write(sock[1], msg, BUFFER_SIZE);
            
            write(p1[1], &logged, sizeof(logged));

            wait(NULL);

            fd=open(FIFO_OUT, O_WRONLY);
            read(sock[1], &outputed_len, sizeof(outputed_len));
            read(sock[1], outputed, BUFFER_SIZE);
            int output_logged, output_running;
            read(p2[0], &output_logged, sizeof(output_logged));
            logged=output_logged;
            running=output_running;
            write(fd, &outputed_len, sizeof(outputed_len));
            write(fd, outputed, BUFFER_SIZE);
            close(sock[1]);
            close(p1[1]);
            close(p2[0]);
            close(fd);
            if(strcmp(msg, "quit")==0)
            {
                break;
            }
            
        
        }
    }

} 



