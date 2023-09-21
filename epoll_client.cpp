#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

using namespace std;
#define MAX_EVENTS 10

int main() {

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  //连接服务器
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(8080); 
  inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
  
  int ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  if(ret != 0){
    cout<<"连接失败"<<ret<<endl;
    return 0;
  }
  int efd = epoll_create(1);

  struct epoll_event event;
  event.data.fd = sockfd;
  event.events = EPOLLIN; // 初始注册可读事件

  epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &event);

  while(1) {

    epoll_wait(efd, &event, 1, -1);

    if(event.events & EPOLLIN) {
      
      char buf[1024];
      int len = read(sockfd, buf, 1024);

      // 处理接收到的数据
      printf("Received: %s\n", buf);

      // 注册 EPOLLOUT 事件,准备发送数据  
      event.data.fd = sockfd;
      event.events = EPOLLOUT;
      epoll_ctl(efd, EPOLL_CTL_MOD, sockfd, &event);

    } else if(event.events & EPOLLOUT) {
    
      const char* req = "GET / HTTP/1.1\r\n\r\n";
      write(sockfd, req, strlen(req));
      
      // 写完后重新注册 EPOLLIN 事件
      event.data.fd = sockfd;  
      event.events = EPOLLIN;
      epoll_ctl(efd, EPOLL_CTL_MOD, sockfd, &event);
    }
  }

  close(sockfd);
  close(efd);

  return 0;
}