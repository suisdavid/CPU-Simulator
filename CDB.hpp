#pragma once
struct CDB{
    int id,dest;
    unsigned int val;
    CDB(int id_=-1,unsigned int val_=0,int dest_=0){
        id=id_;val=val_;dest=dest_;
    }
};