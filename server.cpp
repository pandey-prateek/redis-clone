#include "headers.h"
#include "banner.cpp"
#include "streamoperations.cpp"


static int32_t one_request(int connfd){
    char rbuf[4+k_max_msg+1];
    errno=0;
    int32_t err=read_full(connfd,rbuf,4);
    if(err){
        if(!errno)
            msg("EOF");
        else
            msg("read() error");
        return err;
    }
    uint32_t len=0;
    memcpy(&len,rbuf,4);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }
    err=read_full(connfd,&rbuf[4],len);
    if(err){
        msg("read() error");
        return err;
    }
    rbuf[4+len]='\0';
    std::cout<<"client says: "<<rbuf<<std::endl;


    const char reply[]="somthing somthing";

    char wbuf[4+sizeof(reply)];
    len=(uint32_t)strlen(reply);
    memcpy(wbuf,&len,4);
    memcpy(&wbuf[4],&reply,len);
    return write_all(connfd,wbuf,4+len);
    
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
            int32_t err=one_request(connfd);
            if(err)
                break;
        }
        
        close(connfd);
    }
}
