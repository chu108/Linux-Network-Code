#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/epoll.h>

#define SERV_PORT 6666
#define OPEN_MAX 1024
#define MAXLINE 10

int main(int argc,char **argv)
{
    int ret;                            //作为各个函数的返回值的判断
    char buf[BUFSIZ];
    int connfd;
    //创建listenfd，socket->bind->listen
    int listenfd;
    struct sockaddr_in cliaddr,servaddr;

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    assert( listenfd>=-1 );

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    assert( ret!=-1 );

    ret = listen(listenfd,20);
    assert( ret!=-1 );

    //2.对listenfd进行事件监听
    struct epoll_event retevents[OPEN_MAX];
    struct epoll_event event;
    int epollfd = epoll_create(20);
    //2.1. 将listenfd挂到epollfd树上
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&event);

    //3. 监听listenfd并处理cfd
    while(1)
    {
        int nready = epoll_wait(epollfd,retevents,OPEN_MAX,-1);
        if(nready < 0)
        {
            printf("epoll failure\n");
            exit(1);
        }
        for(int i=0;i<nready;i++)
        {
            int sockfd = retevents[i].data.fd;
            if(sockfd == listenfd)
            {
                //将cfd挂在epollfd树上
                socklen_t clilen = sizeof(cliaddr);
                connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
                event.data.fd = connfd;
                //将connfd文件描述符设置为非阻塞读
                int flag = fcntl(connfd,F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(connfd,F_SETFL,flag);
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&event);
            }else if(retevents[i].events & EPOLLIN)
            {
                int len;
                while((len=read(connfd,buf,MAXLINE/2)) >0 )
                    write(STDOUT_FILENO,buf,len);
            }else{
                printf("something alse happend \n");
            }
        }
    }
    close(listenfd);
    return 0;
}
