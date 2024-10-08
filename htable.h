#ifndef DEFINITIONS
#define DEFINITIONS
#include "headers.h"
struct HNode{
    HNode* next=NULL;
    uint64_t hcode=0;
};

struct HTab{
    HNode **tab = NULL;
    size_t mask =0;
    size_t size=0;
};
struct HMap{
    HTab ht1;
    HTab ht2;
    size_t resizing_pos = 0;
};
struct  Entry
{
    struct HNode node;
    std::string key;
    std::string value;
};

const size_t k_resizing_work = 128;
const size_t k_max_load_factor = 8;
#endif