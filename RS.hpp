#pragma once
#include "op.hpp"
#include "reg.hpp"
struct RS{
    OP_TYPE type;
    unsigned int v1,v2;//value of the two registers
    int q1,q2;//label of the two registers
    int dest,id;
    bool is_halt;
    RS(){
        q1=q2=dest=-1;is_halt=false;
    }
};