#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"

int main(int argc,char **argv)
{
    int cfd;
    struct sockaddr_in cliaddr,servaddr;
    char buf[BUFSIZ];
    cfd=socket(AF_INET,SOCK_STREAM,0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP,&servaddr.sin_addr.s_addr);

    connect(cfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    fgets(buf,sizeof(buf),stdin);
    write(cfd,buf,strlen(buf));
    int n=read(cfd,buf,sizeof(buf));
    write(STDOUT_FILENO,buf,n);

    close(cfd);
    return 0;
}
