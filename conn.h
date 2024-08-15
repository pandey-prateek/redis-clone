#ifndef DEFINITIONS
#define DEFINITIONS
#include "headers.h"
const size_t k_max_msg=4096;
static std::map<std::string,std::string> g_map;
enum {
    STATE_REQ=0,
    STATE_RES=1,
    STATE_END=2,

};
enum {
    RES_OK=0,
    RES_ERR=1,
    RES_NX=2,

};
struct Conn{
    int fd=-1;
    uint32_t state =0;
    size_t rbuf_size=0;
    uint8_t rbuf[4+k_max_msg];
    size_t wbuf_size=0;
    size_t wbuf_sent=0;
    uint8_t wbuf[4+k_max_msg];
};
#endif