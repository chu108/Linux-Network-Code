#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define SERV_PORT 6666

int main(int argc,char *argv[])
{
    int i,j,n,maxi;

    int nready,client[FD_SETSIZE];      /* 自定义数组client，防止遍历1024个文件描述符 */
    int maxfd,listenfd,connfd,sockfd;
    char buf[BUFSIZ],str[INET_ADDRSTRLEN];

    struct sockaddr_in clie_addr,serv_addr;
    socklen_t clie_addr_len;
    fd_set rset,allset;         //该结构体仅包含一个整型数组，数组的每一个元素的每一位标记一个文件描述符。

    listenfd=socket(AF_INET, SOCK_STREAM,0);
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(SERV_PORT);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    listen(listenfd,20);

    maxfd=listenfd;           // 起初listenfd即为最大文件描述符

    maxi=-1;
    for(i=0;i<FD_SETSIZE;i++){
        client[i]=-1;        // 用-1初始化client[]
    }

    FD_ZERO(&allset);           //清除fdset中的所有位
    FD_SET(listenfd,&allset);   //将listenfd设置到fdset

    while(1){
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready<0){
            perror("select error");
            exit(1);
        }
        if(FD_ISSET(listenfd,&rset)){
            clie_addr_len=sizeof(clie_addr);

            connfd=accept(listenfd,(struct sockaddr*)&clie_addr,&clie_addr_len);

            for(i=0;i<FD_SETSIZE;i++){      //遍历client数组，找到一个位置将新建立的socket放进去
                if(client[i]<0){
                    client[i]=connfd;
                    break;
                }
            }
            if(i==FD_SETSIZE){
                fputs("too many clients\n",stderr);
                exit(1);
            }

            FD_SET(connfd,&allset);
            if(connfd>maxfd)
                maxfd=connfd;

            if(i>maxi)
                maxi=i;
            if(--nready == 0){
                continue;
            }
        }

            for(i=0;i<=maxi;i++){                   //依次处理发生事件的socket
                if((sockfd=client[i])<0){
                    continue;
                }
                if(FD_ISSET(sockfd,&rset)){
                    if((n=read(sockfd,buf,sizeof(buf)))==0){
                        close(sockfd);
                        FD_CLR(sockfd,&allset);
                        client[i]=-1;
                    }else if(n > 0){
                        for(j=0;j<n;j++){
                            buf[j]=toupper(buf[j]);
                        }
                        sleep(10);
                        write(sockfd,buf,n);
                    }
                    if(--nready==0)
                        break;
                }
            }
    }
    close(listenfd);
    return 0;
}
