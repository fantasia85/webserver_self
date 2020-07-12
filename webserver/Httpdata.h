/* Http状态机。用于解析命令请求，并发送相应的数据 */

#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <unistd.h>
#include <map>

class Eventloop;
class TimerNode;
class Channel;

/* 当前所在的状态 */
enum Processstate {
    STATE_PARSE_URI = 1,
    STATE_PARSE_HEADERS,
    //STATE_RECV_BODY,
    STATE_ANALYSIS,
    STATE_FINISH
};

enum URIstate {
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS
};

enum Headerstate {
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum Analysisstate {
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR
};

enum Parsestate {
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};

enum Connectionstate {
    H_CONNECTED = 0,
    H_DISCONNECTING,
    H_DISCONNECTED
};

enum Httpmethod {
    METHOD_GET = 1,
    METHOD_HEAD
};

enum Httpversion {
    HTTP_1_0 = 1,
    HTTP_1_1
};

class Mimetype {
    private:
        static void init();
        static std::unordered_map<std::string, std::string> mime;
        
        Mimetype();
        Mimetype(const Mimetype &m);

        static pthread_once_t once_control;

    public:
        static std::string getmime(const std::string &str);
};

class Httpdata : public std::enable_shared_from_this<Httpdata> {
    public:
        Httpdata(Eventloop *loop, int connfd);
        ~Httpdata() {
            close(fd);
        }

        void reset();
        void seperatetimer();
        void linktimer(std::shared_ptr<TimerNode> ptotimer) {
            wptotimer = ptotimer;
        }

        std::shared_ptr<Channel> getchannel() {
            return ptochannel;
        }

        Eventloop *getloop() {
            return loop;
        }

        void handleclose();
        void newevent();

    private:
        Eventloop *loop;
        std::shared_ptr<Channel> ptochannel;
        int fd;
        std::string inbuf;
        std::string outbuf;
        bool iserror;
        Connectionstate connstate;
        Httpmethod hmethod;
        Httpversion hversion;
        std::string flname;
        std::string path;
        int curpos;
        Processstate pcstate;
        Parsestate psstate;
        bool keepalive;
        std::map<std::string, std::string> headers;

        /* use wek_ptr to point to TimerNode. 使用前先判断指针是否被析构，
        * 若未被析构，则提升为shared_ptr，并进行对应的操作 */
        std::weak_ptr<TimerNode> wptotimer;

        void handleread();
        void handlewrite();
        void handleconn();
        void handleerror(int fd, int errnum, std::string err_msg);
        
        URIstate parseURI();
        Headerstate parseheaders();
        Analysisstate analysisrequest();
};