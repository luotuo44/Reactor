#include<unistd.h>

#include <iostream>
#include<string>

#include"Reactor.hpp"
#include"SocketOps.hpp"
#include"Thread.hpp"

using namespace std;
using namespace Net;

Net::EventPtr timer2_ev;
EventPtr timer3_ev;

ReactorPtr reactor;

void stdin_cb(int fd, int events, void *arg)
{
    cout<<"stdin_cb"<<endl;
    char ch[1024];
    int ret = ::read(fd, ch, sizeof(ch));

    int sockfd = *((int*)arg);
    Net::SocketOps::write(sockfd, ch, ret);

    (void)fd;(void)events;
}


void stdin_cb2(int fd, int events, void *arg)
{
    cout<<"in stdin_cb2222222222222"<<endl;
}


void thread_timer_cb(int fd, int events, void *arg)
{
    cout<<"thread_timer_cb"<<endl;

    (void)fd;(void)events; (void)arg;

}

void third_timer_cb(int fd, int events, void *arg)
{
    cout<<"333 timer_cb"<<endl;
}

void thread_fun(void *arg)
{
    EventPtr ev = reactor->createEvent(STDIN_FILENO, EV_READ | EV_PERSIST, stdin_cb2, nullptr);

    Reactor::addEvent(ev);
    Reactor::addEvent(timer3_ev, 500);
}

void timer_cb(int fd, int events, void *arg)
{
    cout<<"first_timer_cb"<<endl;
    (void)fd;(void)events;(void)arg;

    Net::Reactor::delEvent(timer2_ev);

    Thread thread(thread_fun, "test_thread");
    thread.start(nullptr);
}

void second_timer_cb(int fd, int events, void *arg)
{
    cout<<"second timer_cb "<<static_cast<char*>(arg)<<endl;

    (void)fd;(void)events;

}

void socket_read_cb(int fd, int events, void *arg)
{
    char msg[1024];

    //为了简单起见，不考虑读一半数据的情况
    int len = ::read(fd, msg, sizeof(msg)-1);
    if( len <= 0 )
    {
        perror("read fail ");
        exit(1);
    }

    msg[len] = '\0';
    printf("recv %s from server\n", msg);

    (void)events;
}

void timer_read(int fd, int events, void *arg)
{
    if( events & EV_TIMEOUT )
        cout<<"######## EV_TIMEOUT "<<fd<<" "<<reinterpret_cast<char*>(arg)<<endl;
    else
        cout<<"######## EV_READ "<<fd<<" "<<reinterpret_cast<char*>(arg)<<endl;
}

int main()
{
    reactor = Reactor::newReactor();

    int sockfd = SocketOps::new_tcp_socket();
    SocketOps::tcp_connect_server("127.1", 9999, sockfd);
    EventPtr socket_ev = reactor->createEvent(sockfd, EV_PERSIST | EV_READ, socket_read_cb, nullptr);
    Reactor::addEvent(socket_ev);

    EventPtr ev = reactor->createEvent(STDIN_FILENO, EV_PERSIST | EV_READ , stdin_cb, &sockfd);
    EventPtr timer_ev = reactor->createEvent(-1, EV_TIMEOUT, timer_cb, nullptr);

    char str[] = "timer2_ev";
    timer2_ev = reactor->createEvent(-1, EV_TIMEOUT|EV_PERSIST, second_timer_cb, str);
    
    timer3_ev = reactor->createEvent(-1, EV_TIMEOUT|EV_PERSIST, third_timer_cb, nullptr);

	cout<<"create ev, now to add"<<endl;
    Reactor::addEvent(ev);
    Reactor::addEvent(timer_ev, 1200);
    Reactor::addEvent(timer2_ev, 200);


    char arg[] = "arg arg arg";
    Net::EventPtr timer_fd = reactor->createEvent(STDIN_FILENO, EV_TIMEOUT|EV_READ, timer_read, reinterpret_cast<void*>(arg));
    Reactor::addEvent(timer_fd, 10000);

    reactor->dispatch();

    cout << "Hello World!" << endl;
    return 0;
}

