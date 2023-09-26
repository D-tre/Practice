#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#define OPEN_MAX 1024  //最多连接数

int main(int argc, char *argv[])
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    int i;
    int connfd;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));//设置为可重复使用的端口

    struct sockaddr_in serv_addr; //服务器的地址结构体
    memset(&serv_addr,0,sizeof(serv_addr));

    //设置本服务器要监听的地址和端口，这样客户端才能连接到该地址和端口并发送数据
    serv_addr.sin_family = AF_INET;                //选择协议族为IPV4
    serv_addr.sin_port = htons(9000);         //绑定我们自定义的端口号，客户端程序和我们服务器程序通讯时，就要往这个端口连接和传送数据
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //监听本地所有的IP地址；INADDR_ANY表示的是一个服务器上所有的网卡（服务器可能不止一个网卡）多个本地ip地址都进行绑定端口号，进行侦听。

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 32);

    int efd = epoll_create(OPEN_MAX);

    struct epoll_event event, events[OPEN_MAX];
    event.events=EPOLLIN|EPOLLET;
    event.data.fd = listenfd;

    epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event);//把服务器fd包装成事件放在红黑树上

    while(1)
    {
        int nready=epoll_wait(efd,events,OPEN_MAX,-1);//判断为可读事件
        for(i=0; i<nready; i++)
        {
            if(!(events[i].events & EPOLLIN))
            {
                continue;
            }

            if(events[i].data.fd == listenfd)//表示有新的连接
            {
                struct sockaddr_in client_addr;
                int len=sizeof(client_addr);
                memset(&client_addr,0,sizeof(client_addr));
                connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len);
 				event.events = EPOLLIN|EPOLLET; 
				event.data.fd = connfd;  
				epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);               
            }
            else//表示旧的数据产生可读事件(1 客户端发来数据 2 客户端断开链接)
            {
     			connfd=events[i].data.fd;
				char recvline [1024];
				memset(recvline,0,1024);	
                int nread=read(connfd,recvline,sizeof(recvline));
				if(nread==0)
				{
					printf("client is close..\n"); //打印
					epoll_ctl(efd, EPOLL_CTL_DEL, connfd, NULL);//删除果子 select是从集合 和 数组 删除
					close(connfd);//关闭客服端 select一样
                }
				else
				{
					printf("%s",recvline);
				}			
            }
        }
    }

    return 0;    
}
