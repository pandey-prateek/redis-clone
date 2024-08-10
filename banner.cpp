#include "headers.h"

static void die(std::string s){
    std::cerr << s <<std::endl;
    abort();
}

static void msg(std::string s){
    std::cout << s<<std::endl;
}