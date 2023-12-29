#include <iostream>
#include <thread>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> 
#include <string.h>

#endif


#define MAX_LINE 1024
#define MAX_CONN_NUM 1024

int main(int argc, char* argv[])
{
    // linux
#ifdef __linux__
    std::thread t1([](){
        char buf[MAX_LINE], rbuf[MAX_LINE];
        char wbuf[MAX_LINE] = {'w', 'o', 'r', 'l', 'd'};
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in srv_addr;
        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        srv_addr.sin_port = htons(10086);

        struct sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);

        struct epoll_event ev, events[MAX_CONN_NUM];
        int epfd=epoll_create(256);
        ev.data.fd = listenfd;
        ev.events=EPOLLIN|EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

        bind(listenfd, (sockaddr *)&srv_addr, sizeof(srv_addr));
        
        listen(listenfd, MAX_CONN_NUM);
        std::cout << "Srv>> listen addr " << inet_ntop(AF_INET, &srv_addr.sin_addr, buf, sizeof(buf))
                      << ":" << ntohs(srv_addr.sin_port) << std::endl;
        for (;;) 
        {
            int nfds = epoll_wait(epfd, events, MAX_CONN_NUM, -1);

            for (int i = 0; i < nfds; ++i)
            {
                if (events[i].data.fd == listenfd) // new conn
                {
                    ev.data.fd = accept(listenfd, (sockaddr *)&cli_addr, &len);
                    if (ev.data.fd == -1)
                        continue;

                    // set socket noblocking
                    int flags = fcntl(ev.data.fd, F_GETFL, 0);
                    fcntl(ev.data.fd, F_SETFL, flags | O_NONBLOCK);

                    ev.events = EPOLLIN|EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

                    std::cout << "Srv>> build conn from " << inet_ntop(AF_INET, &cli_addr.sin_addr, buf, sizeof(buf))
                              << ":" << ntohs(cli_addr.sin_port) << std::endl;
                }
                else if (events[i].events & EPOLLIN) // data received
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLOUT|EPOLLET;
                    epoll_ctl(ev.data.fd, EPOLL_CTL_MOD, ev.data.fd, &ev);

                    int nrecv = recv(ev.data.fd, rbuf, sizeof(rbuf), 0);
                    std::cout << "Srv>> recv data=" << std::string(rbuf) << std::endl;
                }
                else if (events[i].events & EPOLLOUT) // data need send
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));

                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLIN|EPOLLET;
                    epoll_ctl(ev.data.fd, EPOLL_CTL_MOD, ev.data.fd, &ev);

                    int nsend = send(ev.data.fd, wbuf, sizeof(wbuf), 0);
                    std::cout << "Srv>> send data=" << std::endl;
                }
                else if (events[i].events & EPOLLERR) // error occur, close it
                {
                    std::cout << "Srv>> epoll error!!!" << std::endl;
                    close(events[i].data.fd);
                }
                else if (events[i].events & EPOLLHUP) // sockets be hupped, close it
                {
                    std::cout << "Srv>> socket be hupped!!!" << std::endl;
                    close(events[i].data.fd);
                }
                else
                {
                    continue;
                }
            }
        }

        free(events);
        close(listenfd);
    });
    t1.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t2([](){
        char buf[MAX_LINE]={'h', 'e', 'l', 'l', 'o', '-', 'c', 'l', 'i', '1'};
        int clifd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in srv_addr;
        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        srv_addr.sin_port = htons(10086);

        connect(clifd, (sockaddr *)&srv_addr, sizeof(srv_addr));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        int nsend = send(clifd, buf, sizeof(buf), 0);
        std::cout << "Cli1>> send msg=" << std::string(buf) << std::endl;

        ssize_t nrecv = recv(clifd, buf, sizeof(buf), 0);
        std::cout << "Cli1>> recv msg=" << std::string(buf) << std::endl;

        close(clifd);
    });
    t2.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread t3([](){
        char buf[MAX_LINE]={'h', 'e', 'l', 'l', 'o', '-', 'c', 'l', 'i', '2'};
        int clifd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in srv_addr;
        memset(&srv_addr, 0, sizeof(srv_addr));
        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        srv_addr.sin_port = htons(10086);

        connect(clifd, (sockaddr *)&srv_addr, sizeof(srv_addr));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        int nsend = send(clifd, buf, sizeof(buf), 0);
        std::cout << "Cli2>> send msg=" << std::string(buf) << std::endl;

        ssize_t nrecv = recv(clifd, buf, sizeof(buf), 0);
        std::cout << "Cli2>> recv msg=" << std::string(buf) << std::endl;

        close(clifd);
    });
    t3.detach();

// Other
#else

#endif
    std::cin.get();
    return 0;
}