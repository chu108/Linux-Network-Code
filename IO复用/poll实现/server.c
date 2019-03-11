#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"

int main(int argc, char **argv)
{
    int i,maxi,listenfd,connfd,sockfd;
    int nready;
    ssize_t n;
    char buf[1024];
    socklen_t clilen;
    struct pollfd client[1024];
    struct sockaddr_in cliaddr,servaddr;

    listenfd=socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    int ret = bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    assert(ret != -1);

    ret = listen(listenfd,1024);
    assert(ret != -1);

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for(i =1; i< 1024; i++)
    {
        client[i].fd = -1;
    }
    maxi = 0;
    while(1){
        nready=poll(client,maxi+1,-1);
        if(client[0].revents & POLLRDNORM)
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
            printf("client IP: %s , port: %d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

            for(i=1; i<1024; i++)
            {
                if(client[i].fd < 0)
                {
                    client[i].fd =connfd;
                    break;
                }
            }
            if(i == 1024)
            {
                perror("too many clients\n");
                exit(1);
            }
            if(i > maxi)
                maxi = i;

            if(--nready <= 0)
                continue;
        }

        for(i=1; i<1024; i++)
        {
            if((sockfd = client[i].fd) < 0)
                continue;
            if(client[i].events & (POLLRDNORM | POLLERR))
            {
                if( (n=read(sockfd,buf,1024))<0 )
                {
                    if( errno == ECONNRESET )
                    {
                        close(sockfd);
                        client[i].fd = -1;
                    }else{
                        perror("read error\n");
                        exit(1);
                    }
                }
                else if( n==0 )
                {
                    close(sockfd);
                    client[i].fd= -1;
                }
                else
                {
                    write(sockfd,buf,n);
                    buf[n]='\0';
                    printf("%s\n",buf);
                }
                if(--nready <= 0)
                    break;
            }
        }
    }
    close(listenfd);
    return 0;
}
