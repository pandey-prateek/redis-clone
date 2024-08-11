#include "headers.h"
#include "banner.cpp"
#include "streamoperations.cpp"
// static int32_t one_request(int connfd){
//     char rbuf[4+k_max_msg+1];
//     errno=0;
//     int32_t err=read_full(connfd,rbuf,4);
//     if(err){
//         if(!errno)
//             msg("EOF");
//         else
//             msg("read() error");
//         return err;
//     }
//     uint32_t len=0;
//     memcpy(&len,rbuf,4);
//     if(len > k_max_msg){
//         msg("too long");
//         return -1;
//     }
//     err=read_full(connfd,&rbuf[4],len);
//     if(err){
//         msg("read() error");
//         return err;
//     }
//     rbuf[4+len]='\0';
//     std::cout<<"client says: "<<rbuf<<std::endl;


//     const char reply[]="somthing somthing";

//     char wbuf[4+sizeof(reply)];
//     len=(uint32_t)strlen(reply);
//     memcpy(wbuf,&len,4);
//     memcpy(&wbuf[4],&reply,len);
//     return write_all(connfd,wbuf,4+len);
    
// }
static void fd_set_nb(int fd){
    errno=0;
    int flags=fcntl(fd,F_GETFL,0);
    if(errno){
        die("fcntl error");
        return;
    }
    flags |= O_NONBLOCK;
    errno=0;
    (void)fcntl(fd,F_SETFL,flags);
    if(errno){
        die("fcntl error");
    }
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

static void conn_put(std::vector<Conn*> &fd2conn,struct Conn *conn){
    if(fd2conn.size()<= (size_t)conn->fd){
        fd2conn.resize(conn->fd+1);
    }
    fd2conn[conn->fd]=conn;
}

static int32_t accept_new_conn(std::vector<Conn*>& fd2conn,int fd){
    struct sockaddr_in client_addr={};
        socklen_t socklen=sizeof(client_addr);
        int connfd = accept(fd,(struct sockaddr *)&client_addr,&socklen);
        if(connfd<0){
            msg("accept() error");
            return -1;
        }
        fd_set_nb(connfd);
        struct Conn *conn = new Conn();
        if(!conn){
            close(connfd);
            return -1;
        }
        conn->fd=connfd;
        conn->state=STATE_REQ;
        conn->rbuf_size=0;
        conn->wbuf_size=0;
        conn->wbuf_sent=0;
        conn_put(fd2conn,conn);
        return 0;        
}
static bool try_flush_buffer(Conn* conn){
    ssize_t rv=0;
    do{
        size_t remain = conn->wbuf_size -  conn-> wbuf_sent;
        rv = write(conn->fd,&conn->wbuf[conn->wbuf_sent],remain);
    }while (rv < 0 && errno == EINTR);
    if(rv<0 && errno == EAGAIN)
        return false;
    if(rv<0){
        msg("write() error");
        conn->state = STATE_END;
        return false;
    }
    conn->wbuf_sent +=(size_t)rv;
    assert(conn->wbuf_sent <= conn->wbuf_size);
    if(conn->wbuf_sent == conn->wbuf_size){
        conn->state = STATE_REQ;
        conn->wbuf_sent = 0;
        conn-> wbuf_size =0;
        return false;
    }
    return true;
}


static void state_res(Conn * conn){
    while(try_flush_buffer(conn));
}
static bool try_one_request(Conn* conn){
    if(conn->rbuf_size<4)
        return false;
    uint32_t len=0;
    memcpy(&len,&conn->rbuf[0],4);
    if(len > k_max_msg){
        msg("too long");
        conn->state = STATE_END;
        return false;
    }
    if(4+ len > conn->rbuf_size){
        return false;
    }
    printf("client says: %.*s\n", len, &conn->rbuf[4]);

    memcpy(&conn->wbuf[0],&len,4);
    memcpy(&conn->wbuf[4],&conn->rbuf[4],len);
    conn->wbuf_size = 4+len;
    size_t remain = conn->rbuf_size-4-len;
    if(remain){
        memmove(conn->rbuf,&conn->rbuf[4+len],remain);
    }
    conn->rbuf_size = remain;
    conn->state = STATE_RES;
    state_res(conn);
    return conn->state == STATE_REQ;
}
static bool try_fill_buffer(Conn* conn){
    assert(conn->rbuf_size<sizeof(conn->rbuf));
    ssize_t rv=0;
    do{
        size_t cap= sizeof(conn->rbuf) - conn -> rbuf_size;
        rv=read(conn->fd,&conn->rbuf[conn->rbuf_size],cap);
    }while(rv<0 && errno == EINTR);
    if(rv < 0 && errno == EAGAIN)
        return false;
    if(rv<0){
        msg("read() error");
        conn->state = STATE_END;
        return false;
    }
    if(rv==0){
        if(conn->rbuf_size>0){
            msg("unexpected EOF");
        }else{
            msg("EOF");
        }
        conn->state=STATE_END;
        return false;
    }
    conn->rbuf_size+=(size_t)rv;
    assert(conn->rbuf_size<=sizeof(conn->rbuf) - conn->rbuf_size);
    while(try_one_request(conn));
    return conn->state == STATE_REQ;
}
static void state_req(Conn* conn){
    while(try_fill_buffer(conn));
}
static void connection_io(Conn* conn){
    if(conn->state==STATE_REQ)
        state_req(conn);
    else if(conn->state == STATE_RES)
        state_res(conn);
    else
        assert(0);
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
    std::vector<Conn *> fd2conn;
    fd_set_nb(fd);
    
    std::vector<epoll_event> epev;;
    while(true){
        epev.clear();
            int epfd=epoll_create1(0);
         
        if (epfd < 0) {
               die("epoll create");
        }
        struct epoll_event ev;
        ev.data.fd=fd;
        ev.events=0;
        epev.push_back(ev);
        for(Conn* conn:fd2conn){
            if(!conn)
                continue;
            struct epoll_event ev; 
            ev.data.fd=conn->fd;
            ev.events = EPOLLERR|((conn->state == STATE_REQ) ? EPOLLIN:EPOLLOUT);
            int evfd=epoll_ctl(epfd, EPOLL_CTL_ADD,conn->fd, &ev);
            if(evfd<0)
                die("epoll");
            epev.push_back(ev);        
        }
        int rv=epoll_wait(epfd,epev.data(),(nfds_t)epev.size(), 1000);
        //int rv = poll(pollargs.data(),(nfds_t)pollargs.size(),1000);
        
        if(rv<0)
            die("epoll wait");
        for(size_t i=1;i<epev.size();i++){
            if(epev[i].events){
                Conn* conn=fd2conn[epev[i].data.fd];
                connection_io(conn);
                if(conn->state ==STATE_END){
                    fd2conn[conn->fd] =NULL;
                    (void)close(conn->fd);
                    free(conn);
                }
            }
        }
        if(epev[0].events==EPOLLET){
            (void)accept_new_conn(fd2conn,fd);
        }
        
    }
}
