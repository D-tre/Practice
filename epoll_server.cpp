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
  int efd = epoll_create(1);
  
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);

  // 设置 listen_sock 非阻塞
  fcntl(listen_sock, F_SETFL, O_NONBLOCK);  

int opt = 1;
setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

// 绑定端口
struct sockaddr_in address;
address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(8080); // 指定监听端口8080

bind(listen_sock, (struct sockaddr*)&address, sizeof(address));

  // 将 listen_sock 添加到 epoll 监听
  struct epoll_event listen_event;
  listen_event.events = EPOLLIN;
  epoll_ctl(efd, EPOLL_CTL_ADD, listen_sock, &listen_event);
  struct epoll_event events[MAX_EVENTS];
  while (1) {
    int n = epoll_wait(efd, events, MAX_EVENTS, -1);
    for (int i = 0; i < n; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == listen_sock) {
        cout<<"新连接到来"<<endl;
        int conn_sock = accept(listen_sock, NULL, NULL);
        // 添加到 epoll 继续监听
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = conn_sock;
        epoll_ctl(efd, EPOLL_CTL_ADD, conn_sock, &event);
        
      } else if (events[i].events & EPOLLIN) {
        // 可读事件
        char buf[1024];
        read(sockfd, buf, 1024);
        
      } else if(events[i].events & EPOLLOUT) {
        // 可写事件  
        const char* resp = "HTTP/1.1 200 OK\r\n\r\n";
        write(sockfd, resp, strlen(resp));
        
      } else if(events[i].events & EPOLLHUP) {
        // 关闭连接
        close(sockfd); 
      }
    }

  }
  return 0;
}