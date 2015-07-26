#include<stdio.h>
#include<string.h>
#include<errno.h>

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#include<map>

#include"SocketOps.hpp"
#include"Reactor.hpp"
#include"Logger.hpp"

using namespace Net;

void accept_cb(int fd, int events, void* arg);
void socket_read_cb(int fd, int events, void *arg);

int tcp_server_init(int port, int listen_num);

ReactorPtr reactor;
std::map<int, EventPtr> client_evs;

int main(int argc, char** argv)
{
    int listener = tcp_server_init(9999, 10);
    if( listener == -1 )
    {
        perror(" tcp_server_init error ");
        return -1;
    }

    reactor = Reactor::newReactor();
    EventPtr ev = reactor->createEvent(listener, EV_READ|EV_PERSIST, accept_cb, nullptr);
    Reactor::addEvent(ev);

    reactor->dispatch();

    return 0;
}



void accept_cb(int fd, int events, void* arg)
{
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int sockfd = ::accept(fd, (struct sockaddr*)&client, &len );
    if( sockfd == -1 )
    {
        char buf[50];
        snprintf(buf, sizeof(buf), "accept fail : %m");
        LOG(Log::INFO)<<buf;
        return ;
    }

    printf("accept a client %d\n", sockfd);

    SocketOps::make_nonblocking(sockfd);
    EventPtr ev = reactor->createEvent(sockfd, EV_READ|EV_PERSIST, socket_read_cb, nullptr);
    Reactor::addEvent(ev);

    client_evs.insert({sockfd, ev});
}


void socket_read_cb(int fd, int events, void *arg)
{
    char msg[4096];
    int len = SocketOps::read(fd, msg, sizeof(msg) - 1);

    if( len <= 0 )
    {
        printf("some error happen when read\n");

        SocketOps::close_socket(fd);
        client_evs.erase(fd);
        return ;
    }

    msg[len] = '\0';
    printf("recv the client msg: %s\n", msg);
    fflush(stdout);

    char reply_msg[4096] = "I have recvieced the msg: ";
    strcat(reply_msg + strlen(reply_msg), msg);

    SocketOps::writen(fd, reply_msg, strlen(reply_msg));
}



typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    int listener;

    listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if( listener == -1 )
        return -1;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if( ::bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
        goto error;

    if( ::listen(listener, listen_num) < 0)
        goto error;


    //跨平台统一接口，将套接字设置为非阻塞状态
    SocketOps::make_nonblocking(listener);

    return listener;

    error:
        errno_save = errno;
        SocketOps::close_socket(listener);
        errno = errno_save;

        return -1;
}

