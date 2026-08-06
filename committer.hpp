#pragma once
#include "memory.hpp"
#include "reg.hpp"
#include "op.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "Predictor.hpp"
#include <iostream>
extern Memory memory;
extern Register reg[],pc;
extern bool halt;
extern Predictor predictor;
const int maxk=10;
class Committer{
    private:
        ROB robs[maxk],buffer;
        int l,r,time;
        int debug_cnt;
        bool has_buffer;
    public:
        int total,right;//for branch predicting
        Committer(){
            l=0;r=0;time=0;
            debug_cnt=0;
            has_buffer=0;
            total=right=0;
        }
        bool full(){
            return (r+1)%maxk==l;
        }
        void add(ROB rob){
            buffer=rob;has_buffer=1;
        }
        void update(){
            if (has_buffer)
            {
                robs[r]=buffer;
                r=(r+1)%maxk;
                has_buffer=0;
            }
            if (time){
                time--;
                if (time==0){
                    l=(l+1)%maxk;
                }
            }
        }
        pair<unsigned int,bool>read(int prod)
        {
            pair<unsigned,bool>ans;ans.second=0;
            for (int id=l;id!=r;id=(id+1)%maxk){
                if (robs[id].id==prod){
                    if (robs[id].ready){
                        ans.first=robs[id].value;
                        ans.second=1;
                    }
                    break;
                }
            }
            return ans;
        }

        unsigned long long find(int id,int L,int R){
            unsigned long long val[4]={0xFFFF,0xFFFF,0xFFFF,0xFFFF};
            for (int i=l;i!=r;i=(i+1)%maxk)
            {
                if (robs[i].id>=id){break;}
                if (robs[i].type>0&&robs[i].ready&&robs[i].dest<R&&robs[i].dest+robs[i].type>L){//ready store operations that overlap
                    for (int j=0;j<robs[i].type;j++){
                        int offset=robs[i].dest+j-L;
                        if (offset>=0&&offset<R-L){
                               val[offset]=((robs[i].value>>(8*j))&255);
                        }
                    }
                }
            }
            return val[0]|(val[1]<<16)|(val[2]<<32)|(val[3]<<48);
        }

        pair<int,int> Commit()
        {
            pair<int,int>ans=make_pair(0,0);
            if (!time){
                if (l!=r&&robs[l].ready){
                    time=1;
                    if (robs[l].type>0){//store memory
                        time=2;
                        unsigned int value=robs[l].value;
                        for (int i=0;i<robs[l].type;i++){
                            if (!memory.store(value&255,robs[l].dest+i)){//cache not hit
                                time=3;
                            }
                            value>>=8;
                        }
                       // cout<<"memory["<<cur_rb.dest<<"] changed to "<<cur_rb.value<<"(length "<<cur_rb.type<<")"<<endl;
                        //ans=make_pair(robs[l].id,0);
                    }
                    else if (robs[l].type==0){//alu
                        if (robs[l].dest==(unsigned int)-1){//halt
                            std::cout<<(reg[a0].val&255)<<std::endl;halt=true;
                        }
                        else{
                            reg[robs[l].dest].val=robs[l].value;
                        //  cout<<"reg["<<cur_rb.dest<<"] changed to "<<cur_rb.value<<endl;
                            if (reg[robs[l].dest].prod==robs[l].id){
                                reg[robs[l].dest].prod=-1;
                            }
                        }
                    }
                    else{//branch
                        total++;
                        if (robs[l].value&1){//needs to revert
                            ans=make_pair(-robs[l].id,robs[l].dest);//need to clear all RS > id
                        }
                        else{
                            right++;
                        }
                        predictor.add(-robs[l].type-1,robs[l].value);
                    }
                }
            }
            return ans;
        }
        void receive(CDB cdb){
            for (int id=l;id!=r;id=(id+1)%maxk){
                if (robs[id].id==cdb.id&&robs[id].ready==false){
                    if (cdb.dest!=-2)// -2 is for branch operations, -1 is for halt
                    {
                        robs[id].dest=cdb.dest;
                        robs[id].value=cdb.val;
                    }
                    else
                    {
                        robs[id].value=robs[id].value*2+cdb.val;
                    }
                    robs[id].ready=true;
                }
            }
        }
        void clear(){
            l=r=time=0;
        }
};