#pragma once
#include "reg.hpp"
#include "op.hpp"
extern Register pc;

class Decoder{
    public:
    int id;
    op cur_op;
    Decoder(){
        id=-1;
    }
    Decoder decode(int id,unsigned int x){
        if (id==-1){return Decoder();}
        Decoder new_decoder;
        new_decoder.id=id;
        new_decoder.cur_op=decode(x);
        return new_decoder;
    }
};