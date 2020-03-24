#include"web.h"

int main(int argc,char*argv[])
{
    string buf;
    int n1,n2;
    char content[MAXSIZE];
    buf = getenv("QUERY_STRING");
    int index = buf.find_first_of('&');
    if(index != string::npos)
    {
        n1 = stoi(buf.substr(0,index));
        n2 = stoi(buf.substr(index+1));
    }

    sprintf(content,"QUERY_STRING = %s ",(char*)buf.c_str());
    sprintf(content,"%sWelcome to add.com\r\n",content);
    sprintf(content,"%sThe answer is: %d + %d = %d\r\n",content,n1,n2,n1+n2);
    sprintf(content,"%sThanks for visting\r\n",content);

    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n",(int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s",content);

    return 0;
}