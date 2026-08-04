#pragma once
#include "reg.hpp"
#include "op.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "main_naive.hpp"
#include "BHT.hpp"
#include <iostream>
extern Register reg[],pc;
extern unsigned char memory[];
extern bool halt;
extern BHT bht;
const int maxk=10;
class Committer{
    private:
        ROB robs[maxk],buffer;
        int l,r,time;
        int debug_cnt;
        bool has_buffer;
        void debug_compare(){//for debugging
            debug_cnt++;
            if (naive::do_naive()){
                cout<<"NAIVE ENDED!!!"<<endl;
            }
            int flg=0;
            for (int i=0;i<32;i++){
                if (reg[i].val!=naive::_reg[i].val){
                    flg=1;
                    break;
                }
            }
            if (flg){
                cout<<"NO!!!"<<debug_cnt<<" pc is "<<naive::_pc.val<<endl;
                cout<<"My register values:"<<endl;
                for (int i=0;i<32;i++){
                    cout<<reg[i].val<<" ";
                }
                cout<<endl<<"Standard register values:"<<endl;
                for (int i=0;i<32;i++){
                    cout<<naive::_reg[i].val<<" ";
                }
                cout<<endl;
            }
            else{
                for (int i=0;i<naive::_maxm;i++){
                    if (memory[i]!=naive::_memory[i]){
                        flg=1;break;
                    }
                }
                if (!flg){return;}
                cout<<"NO!!!"<<debug_cnt<<" pc is "<<naive::_pc.val<<endl;
                for (int i=0;i<naive::_maxm;i++){
                    if (memory[i]!=naive::_memory[i]){
                        cout<<"memory differ in "<<i<<", my is "<<(unsigned int)memory[i]<<" ,should be "<<(unsigned int)naive::_memory[i]<<endl;
                    }
                }
            }
        }
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
                    time=(robs[l].type>0?3:1);
                }
            }
            if (time){
                time--;
                if (time==0){
                   // std::cout<<"commit: "<<cur_rb.id<<endl;
                    if (robs[l].type>0){//store memory
                        unsigned int value=robs[l].value;
                        for (int i=0;i<robs[l].type;i++)
                        {
                            memory[robs[l].dest+i]=(value&255);
                            value>>=8;
                        }
                       // cout<<"memory["<<cur_rb.dest<<"] changed to "<<cur_rb.value<<"(length "<<cur_rb.type<<")"<<endl;
                       // debug_compare();
                        ans=make_pair(robs[l].id,0);
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
                       //     cout<<"REVERT TO "<<cur_rb.dest<<endl;
                         //   debug_compare();
                            ans=make_pair(-robs[l].id,robs[l].dest);//need to clear all RS > id
                            bht.add(-robs[l].type-1,((robs[l].value&2)>>1)^1);
                        }
                        else{
                            right++;
                            bht.add(-robs[l].type-1,(robs[l].value&2)>>1);
                        }
                    }
                    l=(l+1)%maxk;
                 // debug_compare();
                }
            }
            return ans;
        }
        void receive(CDB cdb){
            for (int id=l;id!=r;id=(id+1)%maxk){
                if (robs[id].id==cdb.id){
                    if (cdb.dest!=-2)// -2 is for branch operations, -1 is for halt
                    {
                        robs[id].dest=cdb.dest;
                        robs[id].value=cdb.val;
                    }
                    else
                    {
                        robs[id].value=robs[id].value*2+cdb.val;//the LSB means whether need to revert, the second LSB means whether taken
                    }
                    robs[id].ready=true;
                }
            }
        }
        void clear(){
            l=r=time=0;
        }
};