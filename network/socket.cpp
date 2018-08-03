#include "socket.h"
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

BEGIN_NS(network)

static int wait(int socket, int ms);

bool TcpSocket::create(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getAddrinfo(host, service))) {
        return false;
    }
    close();
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if ((_socket = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) {
            continue;
        }
        setOpt(SO_REUSEADDR, 1);
        if (bind(_socket, it->ai_addr, it->ai_addrlen) < 0 || listen(_socket, 5) < 0) {
            close();
            continue;
        }
        return true;
    }
    _errno = errno;
	return false;
}

bool TcpSocket::connect(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getAddrinfo(host, service))) {
        return false;
    }
    close();
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if ((_socket = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) {
            continue;
        }
	    if(::connect(_socket, it->ai_addr, it->ai_addrlen) < 0) {
            close();
	    	continue;
        }
	    return true;
    }
    _errno = errno;
    return false;
}

bool TcpSocket::connect(const char *host, const char *service, int ms) {
	int oldFlags = setNonblock();
	if(!connect(host, service)) {
        if((errno != EINPROGRESS && errno != EWOULDBLOCK) || wait(_socket, ms) <= 0) {
            _errno = errno;
			return false;
        }
	    int error = 0;
        if(getOpt(SO_ERROR, &error) < 0 || error) {
            _errno = errno;
			return false;
        }
	}
	fcntl(_socket, F_SETFL, oldFlags);

	return true;
}

bool TcpSocket::accept(TcpSocket &client) {
	int fd = ::accept(_socket, NULL, NULL);
	if(fd < 0) {
        _errno = errno;
		return false;
    }
	client.attach(fd);
	
    return true;
}

int TcpSocket::recv(void *buf, size_t size, int flags) {
    int n;
    while (true){
	    n = ::recv(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }
    _errno = errno;
	return n;
}

int TcpSocket::send(const void *buf, size_t size, int flags) {
    int n;
    while (true){
	    n = ::send(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }
    _errno = errno;
	return n;
}

int TcpSocket::recvn(void *buf, size_t size, int flags) {
    size_t left = size;
    char *p = (char*)buf;

    while (left > 0) {
        int nr = recv(p, left, flags);
        if (nr <= 0) {
            return nr;
        }
        left -= nr;
        p += nr;
    }
    return size - left;
}

int TcpSocket::sendn(const void *buf, size_t size, int flags) {
    size_t left = size; 
    const char *p = (const char*)buf;

    while (left > 0) {
        int ns = send(p, left, flags);
        if (ns < 0) {
            return ns;
        }
        left -= ns;
        p += ns;
    }
    return size - left;
}

int TcpSocket::setNonblock() {
	int flags = fcntl(_socket, F_GETFL);
	fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
    return flags;
}

void TcpSocket::close() {
	if(_socket != -1) {
		::close(_socket);
		_socket = -1;
	}
}

bool TcpSocket::getpeername(char *addr, int addrlen, int *port) {
    sockaddr_storage peer;
    socklen_t len = sizeof(peer);
    if (::getpeername(_socket, (sockaddr*)&peer, &len) < 0) {
        _errno = errno;
        return false;
    }
    void *inaddr;
    if (peer.ss_family == PF_INET) {
        inaddr = &((sockaddr_in*)&peer)->sin_addr;
        *port = ntohs(((sockaddr_in*)&peer)->sin_port);
    } else if (peer.ss_family == PF_INET6) {
        inaddr = &((sockaddr_in6*)&peer)->sin6_addr;
        *port = ntohs(((sockaddr_in6*)&peer)->sin6_port);
    } else {
        return false;
    }
    if (!inet_ntop(peer.ss_family, inaddr, addr, addrlen)) {
        _errno = errno;
        return false;
    }
    return true;
}

int wait(int socket, int ms)
{
	fd_set rfds;
	struct timeval tv;

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms % 1000) * 1000;
	FD_ZERO(&rfds);
	FD_SET(socket, &rfds);

	int result = select(socket + 1, &rfds, NULL, NULL, &tv);

	if (FD_ISSET(socket, &rfds))
		return result;

	return 0;
}

int UdpSocket::recvfrom(void *buf, size_t size, const char *host, const char *service, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if (ai.getAddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        int sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock < 0) {
            continue;
        }
        return ::recvfrom(sock, buf, size, flags, &_peer, &_len);
    }
    return -1;
}

int UdpSocket::sendto(const void *buf, size_t size, const char *host, const char *service, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if (ai.getAddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        int sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock < 0) {
            continue;
        }
        return ::sendto(sock, buf, size, flags, it->ai_addr, it->ai_addrlen);
    }

    return -1;
}

bool UdpSocket::getpeername(char *addr, int addrlen, int *port) {
    if (!inet_ntop(_peer.sa_family, &_peer, addr, addrlen)) {
        return false;
    }
    *port = ntohs(*(uint16_t*)_peer.sa_data);
    return true;
}

END_NS
