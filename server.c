#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
bool check_redir(char str[])
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '>' || str[i] == '<' )
            return true;
    }
    return false;
}
int main(int argc, char **argv)
{
    int port;
    const int commandsize = 1000;
    printf("What port would you want? ");
    scanf("%d", &port);
    int ssocket, csocket, serverRunning = 1, connected = 0;
    char input[commandsize];
    struct sockaddr_in server, client;
    char* moreCopies,*token;

    if ((ssocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("Some error occured.\n");

    else

        printf("Socket created\n");

    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(ssocket, (struct sockaddr *)&server, sizeof(server)) < 0)
        printf("Bind failed.\n");

    else
        printf("Binding done\n");

    int size = sizeof(struct sockaddr_in);

    while (serverRunning)
    {
        listen(ssocket, 3);
        printf("Awaiting connection.\n");

        csocket = accept(ssocket, (struct sockaddr *)&client, (socklen_t *)&size);

        if (csocket < 0)
        {
            printf("Connection failed.\n");
            return -1;
        }
        else
            printf("Connection accepted.\n");
        connected = 1;
        int msgsize;

        int backuppp = STDOUT_FILENO;

        while (connected)
        {
            strcpy(input, "");
            char buffer[commandsize];
            pid_t pid;

            if ((msgsize = recv(csocket, input, commandsize, 0)) > 0)
            {
                input[msgsize] = '\0';

                moreCopies = strdup(input);
                token = strtok(moreCopies," ");
                
                if (check_redir(input) || strcmp(token,"touch")==0 || strcmp(token,"mv")==0 || strcmp(token,"cp")==0 || strcmp(token,"mkdir")==0)
                {
                    int fd[2];
                    pipe(fd);

                    pid = fork();
                    if (pid == 0)
                    {

                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        execl("./fp", "fp", input, NULL);
                        exit(1);
                    }
                    else
                    {
                        while (wait(NULL)!= pid); 
                        strcpy(buffer, "Done\n");
                        close(fd[1]);
                    }
                    
                } 

                else if (!check_redir(input)) {
                    int fd[2];
                    pipe(fd);

                    pid = fork();
                    if (pid == 0)
                    {

                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        execl("./fp", "fp", input, NULL);
                        exit(1);
                    }
                    else
                    {
                        while (wait(NULL)!= pid); 

                        ssize_t size = read(fd[0], buffer, 1000);
                        if ((size > 0) && (size < sizeof(buffer)))
                        {
                            buffer[size] = '\0';
                        }
                        close(fd[1]);
                    }
                }
                // write(csocket, "Message sent\n", strlen("Message sent\n"));
                printf("\ncommand: (%s)\n", input);

                write(csocket, buffer, strlen(buffer));
                if (strcmp(input, "disconnect") == 0)
                {
                    connected = 0;
                    close(csocket);
                    close(ssocket);
                }
            }
        }
    }
}