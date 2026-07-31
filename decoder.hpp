#pragma once
#include "reg.hpp"
#include "op.hpp"
extern unsigned char memory[];
class Decoder{
    public:
    int id;
    unsigned int opcode;
    Decoder(){
        id=-1;
    }
    pair<int,op> decode(){
        pair<int,op>res;res.first=id;
        if (id!=-1){res.second=op_decode(opcode);}
        return res;
    }
};