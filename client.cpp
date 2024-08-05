#include "headers.h"
#include "banner.cpp"
#include "streamoperations.cpp"


static int32_t query(int fd,const char* text){
    uint32_t len=(uint32_t)strlen(text);
    if(len > k_max_msg)
        return -1;
    char wbuf[4+k_max_msg];
    memcpy(wbuf,&len,4);
    memcpy(&wbuf[4],text,len);

    if(int32_t err=write_all(fd,wbuf,4+len)){
        return err;
    }

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
    printf("server says: %s\n",&rbuf[4]);
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


    int32_t err=query(fd,"hello1");
    if (err) {
        goto L_DONE;
    }
    err=query(fd,"hello2");
    if (err) {
        goto L_DONE;
    }
    
    L_DONE:
        close(fd);
        return 0;
}
