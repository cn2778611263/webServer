#ifndef HTTP_H
#define HTTP_H

#include "httprequest.h"
#define HEAD 0
#define BODY 1
#define HEAD_BODY_OK 3;

int doit(void* request);
int prase_uri(string uri,string& filename,string& cgiargs);
void clienterror(int fd,char* cause,char* errnum,char* shortmsg,char* longmsg);
int http_prase_request_line(http_request* req);
int http_prase_request_body(http_request* req);
void get_filetype(string filename,string& filetype);
void server_static(int fd,string filename,int filesize);
void server_dynamic(int fd,string filename,string cgiargs);

#endif