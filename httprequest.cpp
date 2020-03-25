#include"httprequest.h"

int http_request_init(http_request* hr,int fd,int epfd)
{
    hr->fd = fd;
    hr->epfd = epfd;
    hr->status = 0;
    hr->end = 0;
    hr->keep_alive = false;
    return 0;
}