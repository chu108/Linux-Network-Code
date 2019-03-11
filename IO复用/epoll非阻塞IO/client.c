#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define MAXLINE 10

int main(int argc,char **argv)
{
    char ch='a';
    char buf[MAXLINE];
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in cliaddr,servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);

    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    while(1)
    {
        int i;
        for(i=0;i<MAXLINE/2;i++)
        {
            buf[i] = ch;
        }
        buf[i-1]='\n';
        ch++;
        for(;i<MAXLINE;i++){
            buf[i] = ch;
        }
        buf[i-1]='\n';
        ch++;
        write(sockfd,buf,sizeof(buf));
        sleep(5);
    }
    close(sockfd);
    return 0;
}
