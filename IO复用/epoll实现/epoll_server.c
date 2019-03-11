#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <assert.h>

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"
#define OPEN_MAX  1024
#define MAXLINE 8192

int main(int argc,char **argv)
{
    int i,maxi,listenfd,connfd,sockfd;
    int n,num=0;
    int ret;                //作为各函数的返回判断
    ssize_t nready,efd;
    char buf[MAXLINE],str[INET_ADDRSTRLEN];
    socklen_t clilen;

    struct sockaddr_in cliaddr,servaddr;
    struct epoll_event tep,ep[OPEN_MAX];        //tep为epoll_ctl参数，epoll_wait参数

    listenfd=socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd != -1);

    int opt=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    assert(ret != -1);

    ret = listen(listenfd,20);
    assert(ret != -1);

    efd = epoll_create(OPEN_MAX);
    if(efd == -1)
    {
        perror("epoll_waite error\n");
        exit(1);
    }

    tep.events = EPOLLIN;
    tep.data.fd = listenfd;

    ret = epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);             //将lfd及对应的结构体设置到树上，efd可找到该树
    if(ret == -1)
    {
        perror("epoll_wait error\n");
        exit(1);
    }
    while(1)
    {
        nready = epoll_wait(efd,ep,OPEN_MAX,-1);
        if(nready == -1)
        {
            perror("epoll_wait error\n");
            exit(1);
        }
        for( i=0; i<nready;i++ ){
            if(!(ep[i].events & EPOLLIN))
                continue;

            if(ep[i].data.fd == listenfd)
            {
                clilen = sizeof(cliaddr);
                connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);

                printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),ntohs(cliaddr.sin_port));
                printf("cfd %d----client %d\n",connfd,++num);

                tep.events = EPOLLIN;
                tep.data.fd = connfd;
                ret =epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);
                if(ret == -1)
                {
                    perror("epoll_ctl error\n");
                    exit(1);
                }
            }else{
                sockfd = ep[i].data.fd;
                n = read(sockfd,buf,MAXLINE);

                if(n == 0)
                {
                    ret = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);         //将该文件描述符从红黑树中删除
                    if(ret == -1)
                    {
                        perror("epoll_ctl error\n");
                        exit(1);
                    }
                    close(sockfd);
                    printf("client[%d] closed connection\n",sockfd);
                }else if( n <0 )
                {
                    perror("read n<0 error");
                    ret = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
                    close(sockfd);
                }else{
                    for( i=0;i<n;i++ ){
                        buf[i]=toupper(buf[i]);
                    }
                    write(STDOUT_FILENO,buf,n);
                    write(sockfd,buf,n);
                }
            }
        }
    }


    close(listenfd);
    return 0;
}
