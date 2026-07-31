#pragma once
#include "reg.hpp"
#include "op.hpp"
extern unsigned char memory[];
unsigned int get_op(int id){
    return ((unsigned int)memory[id])|((unsigned int)memory[id+1]<<8)|((unsigned int)memory[id+2]<<16)|((unsigned int)memory[id+3]<<24);
}
class Fetcher{
    public:
    int id;
    unsigned int x;
    Fetcher(){
        id=-1;
    }
    Fetcher fetch(int id){
        Fetcher new_fetcher;
        new_fetcher.id=id;
        new_fetcher.x=get_op(id);
        return new_fetcher;
    }
};
