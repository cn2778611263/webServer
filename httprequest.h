#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "web.h"
#define MAXBUF 4096
struct http_request
{
    int fd;
    int epfd;
    int status;//状态机中的状态设置
    char buf[MAXBUF];
    char method[MAXSIZE];
    char uri[MAXSIZE];
    char version[MAXSIZE];
    int end;//当前读取到的位置
    int pos;//开始位置
    bool keep_alive;
};

int http_request_init(http_request* hr,int fd,int epfd);

#endif