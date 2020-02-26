#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "util/string.h"
#include "httpd/gzip.h"
#include <sys/stat.h>
#include <map>

namespace httpd {

using util::String;
using std::map;
class Request;
class Connection;

class Response : public GCallback {
public:
    enum Status {
        PARSE_REQUEST,
        SEND_HEADERS,
        SEND_CONTENT,
        SEND_DONE,
    };
    Response() {
        reset();
    }
    void parseRequest(const Request &request);
    void setCommonHeaders(const Request &request);
    String headers() const;
    bool sendHeaders(Connection *conn);
    bool sendContent(Connection *conn) {
        return _acceptGz ? sendContentChunked(conn) : sendContentOriginal(conn);
    }
    bool connectionClose() const {
        return _connClose;
    }
    bool is100Continue() const {
        return _code == 100;//ResponseStatus::Continue;
    }
    Status status() const {
        return _status;
    }
    bool inSendHeaders() const {
        return _status == SEND_HEADERS;
    }
    bool inSendContent() const {
        return _status == SEND_CONTENT;
    }
    void reset();

private:
    void setStatusLine(int status, const String &version);
    String parseUri(const String &uri);
    int parseFile(const String &file);
    void setContentInfo(const String &file, const struct stat &st);
    void setContentType(const String &file);
    bool sendContentOriginal(Connection *conn);
    bool sendContentChunked(Connection *conn);
    int gzfill(void *buf, int len);
    bool gzflush(const void *buf, int len, bool eof);

    Connection *_conn;
    String _version;
    int _code;
    String _reason;
    int _fd;
    Status _status;
    off_t _filePos;
    off_t _fileLength;
    uint8_t _cgiBin: 1;
    uint8_t _connClose: 1;
    uint8_t _acceptGz: 1;
    uint8_t _reserve: 5;
    typedef map<int, String>::const_iterator HeaderIt;
    map<int, String> _headers;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_RESPONSE_H__ */
