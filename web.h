#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <map>
#include <string>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <poll.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <signal.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;
#define ERR_RETUNRE(msg) {perror(msg);exit(1);}
#define MAXSIZE 4096
map<string,string> mparsecmdline(int argc,char* argv[]);
void gethost(map<string,string> res,string flag,string &rec,string str);
void getport(map<string,string> res,string flag,int &rec,int def);
int open_listenfd(string shost,int port);
int make_socket_non_blocking(int fd);//将描述符设置为非阻塞