#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("inet_pton failed exit !\n");
        exit(1);
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("connect() failed exit!\n");
        exit(1);        
    }

    int len;
    char recvline[1024]; 
    char writeline[1024];

    while(1)
    {
        memset(recvline, 0, sizeof(recvline));
        memset(writeline, 0, sizeof(writeline));

        //发送消息
		printf("Get From server:\n");
        len = read(sockfd,recvline,1024);

        if(len == 0)
            printf("server is close\n");

        printf("receive from server%d :%s\n",len,recvline);

        printf("send to server\n");
		fgets(writeline,sizeof(writeline),stdin);
		write(sockfd,writeline,strlen(writeline));

        len = read(sockfd,recvline,1024);
    }
    close(sockfd); //关闭套接字
    printf("end exit !\n");
    return 0;     

}
