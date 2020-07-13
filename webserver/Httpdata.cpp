/* Http状态机，用于解析命令请求，并发送相应的数据 */

#include "Httpdata.h"
#include "Channel.h"
#include "Timer.h"
#include "tcpfunc.h"
#include "Eventloop.h"
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

pthread_once_t Mimetype::once_control = PTHREAD_ONCE_INIT;

std::unordered_map<std::string, std::string> Mimetype::mime;

/* 将epoll event通知模式设置为edge triggered.
 * 第一次进行通知，之后不再监测 */
const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;

/* favicon.ico */
char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void Mimetype::init() {
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

std::string Mimetype::getmime(const std::string &str)
{
    pthread_once(&once_control, Mimetype::init);
    if (mime.find(str) == mime.end())
        return mime["default"];
    else
        return mime[str];
}


Httpdata::Httpdata(Eventloop *eloop, int connfd)
    : loop(eloop),
    ptochannel(new Channel(eloop, connfd)),
    fd(connfd),
    iserror(false),
    connstate(H_CONNECTED),
    hmethod(METHOD_GET),
    hversion(HTTP_1_1),
    curpos(0),
    pcstate(STATE_PARSE_URI),
    psstate(H_START),
    keepalive(false)  {
        ptochannel->setreadhandler(std::bind(&Httpdata::handleread, this));
        ptochannel->setwritehandler(std::bind(&Httpdata::handlewrite, this));
        ptochannel->setconnhandler(std::bind(&Httpdata::handleconn, this));
}

void Httpdata::reset() {
    flname.clear();
    path.clear();
    curpos = 0;
    pcstate = STATE_PARSE_URI;
    psstate = H_START;
    headers.clear();
    keepalive = false;
    if (wptotimer.lock()) {
        std::shared_ptr<TimerNode> mytimer(wptotimer.lock());
        mytimer->clearrequest();
        wptotimer.reset(); 
    }
}

void Httpdata::seperatetimer() {
    if (wptotimer.lock()) {
        std::shared_ptr<TimerNode> mytimer(wptotimer.lock());
        mytimer->clearrequest();
        mytimer.reset();
    }
}

void Httpdata::handleread() {
    __uint32_t &events = ptochannel->getevents();
    do {
        bool iszero = false;
        int read_num = readn(fd, inbuf, iszero);
        /*   */
        if (connstate == H_DISCONNECTING) {
            inbuf.clear();
            break;
        }

        if (read_num < 0) {
            perror("1");
            iserror = true;
            handleerror(fd, 400, "Bad Request");
            break;
        }
        else if (iszero) {
            connstate = H_DISCONNECTING;
            if (!read_num)
                break;
        }

        if (pcstate == STATE_PARSE_URI) {
            URIstate flag = this->parseURI();
            if (flag == PARSE_URI_AGAIN)
                break;
            else if (flag == PARSE_URI_ERROR) {
                perror("2");
                /*  */
                inbuf.clear();
                iserror = true;
                handleerror(fd, 400, "Bad Request");
                break;
            }
            else
                pcstate = STATE_PARSE_HEADERS;
        }

        if (pcstate == STATE_PARSE_HEADERS) {
            Headerstate flag = this->parseheaders();
            if (flag == PARSE_HEADER_AGAIN)
                break;
            else if (flag == PARSE_HEADER_ERROR) {
                perror("3");
                iserror = true;
                handleerror(fd, 400, "Bad Request");
                break;
            }

            pcstate = STATE_ANALYSIS;
        }
        
        if (pcstate == STATE_ANALYSIS) {
            Analysisstate flag = this->analysisrequest();
            if (flag == ANALYSIS_SUCCESS) {
                pcstate =STATE_FINISH;
                break;
            }
            else {
                iserror = true;
                break;
            }
        }   
    }while (false);

    if (!iserror) {
        if (outbuf.size() > 0) {
            handlewrite();
        }

        if (!iserror && pcstate == STATE_FINISH) {
            this->reset();
            if (inbuf.size() > 0) {
                if (connstate != H_DISCONNECTING)
                    handleread();
            } 
        }
        else if (!iserror && connstate != H_DISCONNECTED)
            events |= EPOLLIN;
    }
}

void Httpdata::handlewrite() {
    if (!iserror && connstate != H_DISCONNECTED) {
        __uint32_t &events = ptochannel->getevents();
        if (writen(fd, outbuf) < 0) {
            perror("writen");
            events = 0;
            iserror = true;
        }
        if (outbuf.size() > 0)
            events |= EPOLLOUT;
    }
}

void Httpdata::handleconn() {
    seperatetimer();
    __uint32_t &events = ptochannel->getevents();
    if (!iserror && connstate == H_CONNECTED) {
        if (events != 0) {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keepalive) 
                timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events & EPOLLIN) && (events & EPOLLOUT)) {
                events = __uint32_t(0);
                events |= EPOLLOUT;
            }

            events |= EPOLLET;
            loop->updatepoller(ptochannel, timeout);
        }
        else if (keepalive) {
            events |= (EPOLLIN | EPOLLET);
            int timeout =  DEFAULT_KEEP_ALIVE_TIME;
            loop->updatepoller(ptochannel, timeout);
        }       
        else {
            events |= (EPOLLIN | EPOLLET);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop->updatepoller(ptochannel, timeout);
        }
    }
    else if (!iserror && connstate == H_DISCONNECTING && (events &EPOLLOUT)) {
        events = (EPOLLOUT | EPOLLET);
    }
    else {
        loop->runinloop(std::bind(&Httpdata::handleclose, shared_from_this()));
    }
}

/* 解析请求行 */
URIstate Httpdata::parseURI() {
    std::string &str = inbuf;
    std::string cpstr = str;
    size_t pos = str.find('\r', curpos);
    if (pos < 0) {
        return  PARSE_URI_AGAIN;
    }

    std::string requestline = str.substr(0, pos);
    if (str.size() > pos + 1)
        str = str.substr(pos + 1);
    else
        str.clear();

    int posget = requestline.find("GET");
    int poshead = requestline.find("HEAD");

    if (posget >= 0) {
        pos = posget;
        hmethod = METHOD_GET;
    }
    else if (poshead >= 0) {
        pos = poshead;
        hmethod = METHOD_HEAD;
    }
    else 
        return PARSE_URI_ERROR;

    pos = requestline.find("/", pos);
    if (pos < 0) {
        flname = "index.html";
        hversion = HTTP_1_1;
        return PARSE_URI_SUCCESS;
    }
    else {
        size_t pos1 = requestline.find(' ', pos);
        if (pos1 < 0)
            return PARSE_URI_ERROR;
        else {
            if (pos1 - pos > 1) {
                flname = requestline.substr(pos + 1, pos1 - pos - 1);
                size_t pos2 = flname.find('?');
                if (pos2 >= 0) {
                    flname = flname.substr(0, pos2);
                }
            }
            else
                flname = "index.html";
        }
        pos = pos1;
    }

    pos = requestline.find("/", pos);
    if (pos < 0)
        return PARSE_URI_ERROR;
    else {
        if (requestline.size() - pos <= 3)
            return PARSE_URI_ERROR;
        else {
            std::string vsion = requestline.substr(pos + 1, 3);
            if (vsion == "1.0")
                hversion = HTTP_1_0;
            else if (vsion == "1.1")
                hversion = HTTP_1_1;
            else
                return PARSE_URI_ERROR;
        }
    }
    return PARSE_URI_SUCCESS;
}

/* 解析请求头部 */
Headerstate Httpdata::parseheaders() {
    std::string &str = inbuf;
    int key_start = -1, key_end = -1;
    int val_start = -1, val_end = -1;
    int readbegin = 0;
    bool notfinish = true;
    size_t i = 0;
    for ( ; i < str.size() && notfinish; ++i) {
        switch (psstate) {
            case H_START:
                if (str[i] == '\n' || str[i] == '\r')
                    break;
                psstate = H_KEY;
                key_start = i;
                readbegin = i;
                break;
            case H_KEY:
                if (str[i] == ':') {
                    key_end = i;
                    if (key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    psstate = H_COLON;
                }
                else if (str[i] == '\n' || str[i] == '\r')
                    return PARSE_HEADER_ERROR;
                break;
            case H_COLON: 
                if (str[i] == ' ')
                    psstate = H_SPACES_AFTER_COLON;
                else
                    return PARSE_HEADER_ERROR;
                break;
            case H_SPACES_AFTER_COLON:
                psstate = H_VALUE;
                val_start = i;
                break;
            case H_VALUE:
                if (str[i] == '\r') {
                    psstate = H_CR;
                    val_end = i;
                    if (val_end - val_start <= 0)
                        return PARSE_HEADER_ERROR;
                    else if (i - val_start > 255)
                        return PARSE_HEADER_ERROR;
                    break;
                }
            case H_CR:
                if (str[i] == '\n') {
                    psstate = H_LF;
                    std::string key(str.begin() + key_start, str.begin() + key_end);
                    std::string value(str.begin() + val_start, str.begin() + val_end);
                    headers[key] = value;
                    readbegin = i;
                }
                else 
                    return PARSE_HEADER_ERROR;
                break;
            case H_LF:
                if (str[i] == '\r')
                    psstate = H_END_CR;
                else {
                    key_start = i;
                    psstate = H_KEY;
                }
                break;
            case H_END_CR:
                if (str[i] == '\n')
                    psstate = H_END_LF;
                else
                    return PARSE_HEADER_ERROR;
                break;
            case H_END_LF:
                notfinish = false;
                key_start = i;
                readbegin = i;
                break;
        }
    }
    if (psstate == H_END_LF) {
        str = str.substr(i);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(readbegin);
    return PARSE_HEADER_AGAIN;
}

/* 解析请求内容 */
Analysisstate Httpdata::analysisrequest() {
    if (hmethod == METHOD_GET || hmethod == METHOD_HEAD) {
        std::string header;
        header += "HTTP/1.1 200 OK\r\n";
        if (headers.find("Connection") != headers.end() &&
           (headers["connection"] == "Keep-Alive" || headers["Connection"] == "Keep-alive")) {
               keepalive = true;
               header += std::string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout= "
                            + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
        }
        int dotpos = flname.find('.');
        std::string fltype;
        if (dotpos < 0)
            fltype = Mimetype::getmime("default");
        else
            fltype = Mimetype::getmime(flname.substr(dotpos));

        if (flname == "hello") {
            outbuf = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
            return ANALYSIS_SUCCESS;
        }
        if (flname == "favicon.ico") {
            header += "Content-Type: image/png\r\n";
            header += "Content-Length: " + std::to_string(sizeof(favicon)) + "\r\n";
            header += "Server: Web Server\r\n";

            header += "\r\n";
            outbuf += header;
            outbuf += std::string(favicon, favicon + sizeof(favicon));
            return ANALYSIS_SUCCESS;
        }

        struct stat sbuf;
        if (stat(flname.c_str(), &sbuf) < 0) {
            header.clear();
            handleerror(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        header += "Content-Type: " + fltype + "\r\n";
        header += "Content-Length" + std::to_string(sbuf.st_size) + "\r\n";
        header += "Server: Web Server\r\n";
        header += "\r\n";
        outbuf += header;

        if (hmethod == METHOD_HEAD)
            return ANALYSIS_SUCCESS;

        int srcfd = open(flname.c_str(), O_RDONLY, 0);
        if (srcfd < 0) { 
            outbuf.clear();
            handleerror(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        void *mmapret = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
        close(srcfd);
        if (mmapret == (void *) - 1) {
            munmap(mmapret, sbuf.st_size);
            outbuf.clear();
            handleerror(fd, 404, "Not Found!");
            return ANALYSIS_SUCCESS;
        }
        char *src_addr = static_cast<char *>(mmapret);
        outbuf += std::string(src_addr, src_addr + sbuf.st_size);
        munmap(mmapret, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    return ANALYSIS_ERROR;
}

void Httpdata::handleerror(int fd, int errnum, std::string err_msg) {
    err_msg = " " + err_msg;
    char sendbuf[4096];
    std::string bodybuf, headerbuf;
    bodybuf += "<html><title>哎呀~出错了~</title>";
    bodybuf += "<body bgcolor=\"ffffff\">";
    bodybuf += std::to_string(errnum) + err_msg;
    bodybuf += "<hr><em> Web Server</em>\n</body></html>";

    headerbuf += "HTTP/1.1 " + std::to_string(errnum) + err_msg;
    headerbuf += "Content-Type: text/html\r\n";
    headerbuf += "Connection: Close\r\n";
    headerbuf += "Content-Length: " + std::to_string (bodybuf.size()) + "\r\n";
    headerbuf += "Server: Web Server\r\n";
    headerbuf = "\r\n";

    sprintf(sendbuf, "%s", headerbuf.c_str());
    writen(fd, sendbuf, strlen(sendbuf));
    sprintf(sendbuf, "%s", bodybuf.c_str());
    writen(fd, sendbuf, strlen(sendbuf));
}

void Httpdata::handleclose() {
    connstate = H_DISCONNECTED;
    std::shared_ptr<Httpdata> guard(shared_from_this());
    loop->removefrompoller(ptochannel);
}

void Httpdata::newevent() {
    ptochannel->setevents(DEFAULT_EVENT);
    loop->addtopoller(ptochannel, DEFAULT_EXPIRED_TIME);
}