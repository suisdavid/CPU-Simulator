#pragma once
#include "reg.hpp"
#include "op.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "main_naive.hpp"
#include <iostream>
extern Register reg[],pc;
extern unsigned char memory[];
extern bool halt;
const int maxk=1024;
class Committer{
    private:
        ROB robs[maxk],cur_rb,buffer;
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
        Committer(){
            l=0;r=0;time=0;
            debug_cnt=0;
            has_buffer=0;
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
        pair<int,int> Commit()
        {
            if (!time){
                if (l!=r&&robs[l].ready){
                    cur_rb=robs[l];
                    l=(l+1)%maxk;
                    time=(cur_rb.type>0?3:1);
                }
            }
            if (time){
                time--;
                if (time==0){
                   // std::cout<<"commit: "<<cur_rb.id<<endl;
                    if (cur_rb.type>0){//store memory
                        unsigned int value=cur_rb.value;
                        for (int i=0;i<cur_rb.type;i++)
                        {
                            memory[cur_rb.dest+i]=(value&255);
                            value>>=8;
                        }
                     //   cout<<"memory["<<cur_rb.dest<<"] changed to "<<cur_rb.value<<"(length "<<cur_rb.type<<")"<<endl;
                       // debug_compare();
                        return make_pair(cur_rb.id,0);
                    }
                    else if (cur_rb.type==0){//alu
                        if (cur_rb.dest==(unsigned int)-1){
                            std::cout<<(reg[a0].val&255)<<std::endl;halt=true;
                            return make_pair(0,0);
                        }
                        reg[cur_rb.dest].val=cur_rb.value;
                     //    cout<<"reg["<<cur_rb.dest<<"] changed to "<<cur_rb.value<<endl;
                        if (reg[cur_rb.dest].prod==cur_rb.id){
                            reg[cur_rb.dest].prod=-1;
                        }
                    }
                    else{//branch
                        if (cur_rb.value){//needs to revert
                       //     cout<<"REVERT TO "<<cur_rb.dest<<endl;
                          //  debug_compare();
                            return make_pair(-cur_rb.id,cur_rb.dest);//need to clear all RS > id
                        }
                    }
                  //debug_compare();
                }
            }
            return make_pair(0,0);
        }
        void receive(CDB cdb){
            if (has_buffer&&buffer.id==cdb.id){
                if (cdb.dest!=-2)// -2 is for branch operations, -1 is for halt
                {
                    buffer.dest=cdb.dest;
                }
                buffer.value=cdb.val;
                buffer.ready=true;
            }
            for (int id=l;id!=r;id=(id+1)%maxk){
                if (robs[id].id==cdb.id){
                    if (cdb.dest!=-2)// -2 is for branch operations, -1 is for halt
                    {
                        robs[id].dest=cdb.dest;
                    }
                    robs[id].value=cdb.val;
                    robs[id].ready=true;
                }
            }
        }
        void clear(){
            l=r=time=0;
        }
};