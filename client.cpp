#include "headers.h"
#include "banner.cpp"
#include "streamoperations.cpp"
static int32_t send_req(int fd, const char *text){
    uint32_t len=(uint32_t)strlen(text);
    if(len > k_max_msg)
        return -1;
    char wbuf[4+k_max_msg];
    memcpy(wbuf,&len,4);
    memcpy(&wbuf[4],text,len);

    return write_all(fd,wbuf,4+len);
}
static int32_t read_res(int fd){
    char rbuf[4+k_max_msg+1];
    errno=0;
    int32_t err=read_full(fd,rbuf,4);
    if(err){
        if(!errno)
            msg("EOF");
        else
            msg("read() error");
        return err;
    }
    uint32_t len = 0;
    memcpy(&len,rbuf,4);
    if(len>k_max_msg){
        msg("too long");
        return -1;
    }
    err=read_full(fd,&rbuf[4],len);
    if (err) {
        msg("read() error");
        return err;
    }
    rbuf[4+len]='\0';
    std::cout<<"server says: "<<(&rbuf[4])<<std::endl;
    return 0;
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
    const char* query_list[3] = {"hello1","hello2","hello3"};
    for(size_t i=0;i<3;i++){
        int32_t err=send_req(fd,query_list[i]);
        if(err)
            goto L_DONE;
    }
    for(size_t i=0;i<3;i++){
        int32_t err=read_res(fd);
        if(err)
            goto L_DONE;
    }
    L_DONE:
        close(fd);
}
