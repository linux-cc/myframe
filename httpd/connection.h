#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "util/buffer_queue.h"
#include "util/string.h"
#include "util/simple_list.h"
#include "util/scoped_ptr.h"
#include "memory/simple_alloc.h"
#include "network/socket.h"
#include "httpd/request.h"
#include "httpd/response.h"

namespace httpd {

using util::SimpleList;
using util::BufferQueue;
using util::ScopedPtr;
using memory::SimpleAlloc;
using network::TcpSocket;
class Request;
class Response;
class Connection {
public:
    Connection(int socket = -1): _socket(socket), _toNewIdx(-1), _toOldIdx(-1) {}
    bool recv();
    size_t recv(void *buf, size_t size) { return _recvQ.dequeue(buf, size); }
    bool recvLine(String &buf) { return _recvQ.dequeueUntil(buf, ONE_CRLF, false); }
    bool recvUntil(String &buf, const char *pattern, bool flush) { return _recvQ.dequeueUntil(buf, pattern, flush); }
    
    ssize_t send(const String &buf) { return send(buf.data(), buf.length()); }
    ssize_t send(const void *buf, size_t size);
    ssize_t send(struct iovec *iov, int iovcnt);
    void close() { _socket.close(); }
    bool isClosed() const { return _socket.isClosed(); }

    int fd() const { return _socket; }
    void attach(int fd) { _socket.attach(fd); }
    
    Request *getRequest() { return _req.release(); }
    void setRequest(Request *req) { _req.reset(req); }
    void addRequest(Request *req) { _reqList.push_back(req); }
    Request *popRequest();
    Response *getResponse() { return _resp.release(); }
    void setResponse(Response *resp) { _resp.reset(resp); }
    
    void setNewIdx(int newIdx) { _toNewIdx = newIdx; }
    void setOldIdx(int oldIdx) { _toOldIdx = oldIdx; }
    int getNewIdx() const { return _toNewIdx; }
    int getOldIdx() const { return _toOldIdx; }
    
private:
    BufferQueue _recvQ;
    BufferQueue _sendQ;
    ScopedPtr<Request> _req;
    ScopedPtr<Response> _resp;
    SimpleList<Request*> _reqList;
    TcpSocket _socket;
    int _toNewIdx;
    int _toOldIdx;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONNECT_H */
