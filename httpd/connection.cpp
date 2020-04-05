#include "httpd/connection.h"
#include <errno.h>

namespace httpd {

bool Connection::recv() {    
    return _recvBuf.refill() > 0 || errno == EAGAIN;
}

bool Connection::recvUntil(String &buf, const char *pattern, bool flush) {
    size_t p = _recvBuf.find(pattern);
    if (p == -1) {
        if (flush) {
            buf.resize(_recvBuf.length());
            _recvBuf.read((char*)buf.data(), buf.length());
        }
        return false;
    }
    buf.resize(p + strlen(pattern));

    return _recvBuf.read((char*)buf.data(), buf.length()) == buf.length();
}

ssize_t Connection::send(const void *buf, size_t size) {
    if (!_sendBuf.empty()) {
        return _sendBuf.write(buf, size);
    }

    ssize_t n = _socket.send(buf, size);
    if (n < 0 && errno != EAGAIN) {
        return n;
    }
    
    if (n < size) {
        n += _sendBuf.write((char*)buf + n, size - n);
    }
    
    return n;
}

ssize_t Connection::send(struct iovec *iov, int iovcnt) {
    if (!_sendBuf.empty()) {
        ssize_t n = 0;
        for (int i = 0; i < iovcnt; i++) {
            ssize_t s = _sendBuf.write(iov[i].iov_base, iov[i].iov_len);
            if (s < 0) {
                return s;
            }
            n += s;
        }
        return n;
    }

    ssize_t n = _socket.send(iov, iovcnt);
    size_t total = 0;
    int i = 0;
    for (; i < iovcnt; ++i) {
        total += iov[i].iov_len;
        if (n < total) break;
    }

    size_t rest = total - n;
    for (int j = i; j < iovcnt; j++) {
        void *base = (char*)iov[j].iov_base + (j == i ? iov[j].iov_len - rest : 0);
        ssize_t w = _sendBuf.write(base, j == i ? rest : iov[j].iov_len);
        if (w < 0) {
            return w;
        }
        n += w;
    }
    
    return n;
}

Request *Connection::popRequest() {
    if (_reqList.empty()) {
        return NULL;
    }
    
    Request *req = _reqList.front();
    _reqList.popFront();
    return req;
}

} /* namespace httpd */
