#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

void str_cli(FILE *fp, int sockfd)
{
    printf("Connected sucessfully! \n");

    int bufferSize = 4096;
    char input[bufferSize], returned[bufferSize];
    
    while (fgets(input, bufferSize, fp) != NULL)
    {
        write(sockfd, input, strlen(input)-1);
        read(sockfd, returned, bufferSize);
        fputs(returned, stdout);
        bzero(returned, bufferSize);
    }
}

int main(int argc, char **argv)
{
    int socketfiledescriptor,status;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    socketfiledescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfiledescriptor == -1)
        printf("socket error\n");

    status = connect(socketfiledescriptor, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    if (socketfiledescriptor == -1)
        printf("connection error\n");

    str_cli(stdin, socketfiledescriptor);

    exit(0);
}

