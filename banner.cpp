#include "headers.h"

static void die(std::string s){
    std::cerr << s;
    abort();
}

static void msg(std::string s){
    std::cout << s;
}