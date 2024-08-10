#include "headers.h"
static int32_t read_full(int fd,char* buf,size_t n){
    while(n>0){
        ssize_t rv = read(fd,buf,n);
        if(rv<=0){
            return -1;
        } 
        assert((size_t)rv<=n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}
static int32_t write_all(int fd,const char* buf,size_t n){
    while (n>0)
    {
        /* code */
        ssize_t rv = write(fd,buf,n);
        if(rv<=0){
            return -1;
        } 
        assert((size_t)rv<=n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}