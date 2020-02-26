#include "network/socket.h"
#include "network/epoll.h"
#include <errno.h>
#include <vector>

using std::vector;
using namespace network;

static bool __quit = false;
#ifdef __TEST__
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage %s port ipversion[0|1|2|3]\n", argv[0]);
        return -1;
    }
    TcpSocket::Family family;
    switch(argv[2][0]) {
    case '1': family = TcpSocket::F_INET4; break;
    case '2': family = TcpSocket::F_INET6; break;
    case '3': family = TcpSocket::F_LOCAL; break;
    default: family = TcpSocket::F_UNSPEC;
    }
    TcpSocket server;
    EPoller poller;
    if (!server.create("localhost", argv[1], family)) {
        printf("server create error: %d:%s\n", errno, strerror(errno));
        return -1;
    }
    if (!poller.create(1024)) {
        printf("poller create failed\n");
        return -1;
    }
    printf("listen in port %s\n", argv[1]);
    server.setNonblock();
    poller.add(server);
    char buf[1024];
    while (!__quit) {
        EPollResult result = poller.wait(1000);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->fd() == server) {
                while (1) {
                    int fd = server.accept();
                    if (fd < 0) {
                        if (errno != EWOULDBLOCK && errno != EAGAIN)
                            printf("server accept error: %d:%s\n", errno, strerror(errno));
                        poller.mod(server);
                        break;
                    }
                    TcpSocket client(fd);
                    client.setNonblock();
                    poller.add(client);
                    Peername peer = client.getPeerName();
                    printf("server accept: [%s|%d]\n", peer.name(), peer.port());
                }
            } else {
                TcpSocket client(it->fd());
                Peername peer = client.getPeerName();
                int len = client.recv(buf, 1024);
                if (len <= 0) {
                    printf("client[%s|%d]: close, len:%d\n", peer.name(), peer.port(), len);
                    poller.del(client);
                    client.close();
                } else {
                    buf[len] = 0;
                    printf("receive data[%s|%d]: %s", peer.name(), peer.port(), buf);
                    client.send(buf, len);
                    poller.mod(client);
                }
            }
        }
    }

    return 0;
}
#endif
