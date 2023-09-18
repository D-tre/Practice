#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {

  int efd = epoll_create(1);
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  int conn_sock = accept(listen_sock, NULL, NULL);

  struct epoll_event event;
  event.data.fd = conn_sock;
  event.events = EPOLLIN; // initial event: read

  epoll_ctl(efd, EPOLL_CTL_ADD, conn_sock, &event);

  while(1) {

    int n = epoll_wait(efd, &event, 1, -1);

    if(event.events & EPOLLIN) { // ready for read
      
      char buf[1024];
      read(conn_sock, buf, 1024); // read data

    } else if(event.events & EPOLLOUT) { // ready for write

      const char* resp = "HTTP/1.1 200 OK\r\n\r\n";
      write(conn_sock, resp, strlen(resp)); // write response

    } else if(event.events & EPOLLHUP) { // socket closed
      
      close(conn_sock); 
      break;

    }

    // change events for next step
    event.data.fd = conn_sock;  
    event.events = EPOLLOUT; // now wait for write
    epoll_ctl(efd, EPOLL_CTL_MOD, conn_sock, &event);

  }

  close(efd);

  return 0; 
}
