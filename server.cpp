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
static void do_somthing(int connfd){
    char rbuf[64]={};
    ssize_t n=read(connfd,rbuf,sizeof(rbuf)-1);
    if(n<0){
        msg("read() error");
        return;
    }
    std::cout<<"client says: "<<rbuf<<std::endl;
    std::string wbuf="world";
    write(connfd,wbuf.c_str(),wbuf.size());
}

int main(int argc, char const *argv[])
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
        die("socket()");
    int val=1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

    struct sockaddr_in add={};
    add.sin_family=AF_INET;
    add.sin_port=ntohs(1234);
    add.sin_addr.s_addr=ntohl(0);
    int rv=bind(fd,(const sockaddr *)&add,sizeof(add));
    if(rv){
        die("bind()");
    }
    rv=listen(fd,SOMAXCONN);
    if(rv)
        die("listen()");
    while(true){
        struct sockaddr_in client_addr={};
        socklen_t socklen=sizeof(client_addr);
        int connfd= accept(fd,(struct sockaddr *)&client_addr,&socklen);
        if(connfd<0)
            continue;
        while(true){
            int32_t err=do_somthing(connfd);
            if(err)
                break;
        }
        
        close(connfd);
    }
}
