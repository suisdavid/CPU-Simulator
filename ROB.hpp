#pragma once
#include "op.hpp"
#include "reg.hpp"
struct ROB{
    int id;
    unsigned int value,dest;
    int type;//if store memory: type=1/2/4; if branch: type=-1;otherwise type=0
    bool ready;
    ROB(int id_=-1,unsigned int dest_=0,int type_=0,unsigned int value_=0,bool ready_=0){
        id=id_;dest=dest_;type=type_;value=value_;ready=ready_;
    }
};