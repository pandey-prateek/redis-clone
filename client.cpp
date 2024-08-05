#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>
#include <iostream>

static void die(std::string s){
    std::cerr << s;
    abort();
}
static void msg(std::string s){
    std::cout << s;
}
int main(int argc, char const *argv[])
{
    /* code */
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
        die("socket()");
    
    struct sockaddr_in add={};
    add.sin_family=AF_INET;
    add.sin_port=ntohs(1234);
    add.sin_addr.s_addr=ntohl(INADDR_LOOPBACK);
    int rv=connect(fd,(const struct sockaddr*)&add,sizeof(add));
    if(rv)
        die("connect");
    std::string msg="hello";
    write(fd,msg.c_str(),msg.size());
    char recv_buf[64]={};
    ssize_t n=read(fd,recv_buf,sizeof(recv_buf)-1);
    if(n<0)
        die("read");
    std::cout<<"Server says "<<recv_buf<<std::endl;
    close(fd);
}
