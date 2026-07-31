#pragma once
#include "reg.hpp"
#include "op.hpp"
#include "RS.hpp"
#include "CDB.hpp"
#include "committer.hpp"
extern unsigned char memory[];
extern Register reg[],pc;
const int maxm=1024;
int signed12(unsigned int x){
    if (x<2048){return (int)x;}
    return (int)x-4096;
}

int signed13(unsigned int x){
    if (x<4096){return (int)x;}
    return (int)x-8192;
}

int signed21(unsigned int x){
    if (x<1048576){return (int)x;}
    return (int)x-2097152;
}

bool is_load(OP_TYPE type){
    return type==OP_LB||type==OP_LBU||type==OP_LH||type==OP_LHU||type==OP_LW;
}

bool is_store(OP_TYPE type){
    return type==OP_SB||type==OP_SH||type==OP_SW;
}

class Executer{
    protected:
        RS rss[maxm],cur_rs,buffer;
        int l,r,time;
        bool has_buffer;
        Executer(){
            l=0;r=0;time=0;has_buffer=0;
        }
        bool run(){
            if (!time){
                for (int id=l;id!=r;id=(id+1)%maxm){//find the first ready instruction
                    if (rss[id].q1==-1&&rss[id].q2==-1){
                        cur_rs=rss[id];
                        time=(is_load(cur_rs.type)?3:1);
                        for (int i=id;i!=l;i=(i+maxm-1)%maxm){
                            rss[i]=rss[(i+maxm-1)%maxm];
                        }
                        l=(l+1)%maxm;
                        break;
                    }
                    if (is_store(rss[id].type)){//store op not completed
                        return 0;
                    }
                }
            }
            if (time){
                time--;
                return time==0;
            }
            return 0;
        }
    public:
        bool full(){
            return (r+1)%maxm==l;
        }
        void add(RS rs){
            buffer=rs;has_buffer=1;
        }
        void receive(CDB cdb){
            if (has_buffer){
                if (buffer.q1==cdb.id){
                    buffer.q1=-1;buffer.v1=cdb.val;
                }
                if (buffer.q2==cdb.id){
                    buffer.q2=-1;buffer.v2=cdb.val;
                }
            }
            for (int i=l;i!=r;i=(i+1)%maxm){
                if (rss[i].q1==cdb.id){
                    rss[i].q1=-1;rss[i].v1=cdb.val;
                }
                if (rss[i].q2==cdb.id){
                    rss[i].q2=-1;rss[i].v2=cdb.val;
                }
            }
        }
        void clear(int id){
            while (r!=l&&rss[(r+maxm-1)%maxm].id>id){
                r=(r+maxm-1)%maxm;
            }
            if (r==l){
                time=0;//completely cleared
            }
        }
        void update(){
            if (has_buffer)
            {
                rss[r]=buffer;
                r=(r+1)%maxm;
                has_buffer=0;
            }
        }
};

class ALU: public Executer{
    public:
        CDB execute(){//return value is CDB broadcast
           if (run()){
                unsigned int val=0,r1=cur_rs.v1,r2=cur_rs.v2;
                int dest=(cur_rs.is_halt?-1:cur_rs.dest);
                switch (cur_rs.type){
                    case OP_ADD:
                        val=r1+r2;
                        break;
                    case OP_SUB:
                        val=r1-r2;
                        break;
                    case OP_AND:
                        val=(r1&r2);
                        break;
                    case OP_OR:
                        val=(r1|r2);
                        break;
                    case OP_XOR:
                        val=(r1^r2);
                        break;
                    case OP_SLL:
                        val=(r1<<(r2&31));
                        break;
                    case OP_SRL:
                        val=(r1>>(r2&31));
                        break;
                    case OP_SRA:
                        val=((signed int)r1>>(r2&31));
                        break;
                    case OP_SLT:
                        val=((signed int)r1<(signed int)r2?1:0);
                        break;
                    case OP_SLTU:
                        val=(r1<r2?1:0);
                        break;
                    case OP_ADDI:
                        val=r1+signed12(r2);
                        break;
                    case OP_ANDI:
                        val=(r1&signed12(r2));
                        break;
                    case OP_ORI:
                        val=(r1|signed12(r2));
                        break;
                    case OP_XORI:
                        val=(r1^signed12(r2));
                        break;
                    case OP_SLLI:
                        val=(r1<<r2);
                        break;
                    case OP_SRLI:
                        val=(r1>>r2);
                        break;
                    case OP_SRAI:
                        val=((signed int)r1>>r2);
                        break;
                    case OP_SLTI:
                        val=((signed int)r1<signed12(r2)?1:0);
                        break;
                    case OP_SLTIU:
                        val=(r1<(unsigned int)signed12(r2)?1:0);
                        break;
                    case OP_JAL:
                    case OP_JALR:
                    case OP_AUIPC:
                    case OP_LUI:
                        val=r1;
                        break;
                }
                return CDB(cur_rs.id,val,dest);
           }
           return CDB();
        }   
};

class LSU: public Executer{
    public:
        int lst_store;
        LSU(){
            lst_store=-1;
        }
        CDB execute(){
            if (lst_store==-1&&run())
            {
                if (is_store(cur_rs.type)){
                    lst_store=cur_rs.id;
                }
                unsigned int val=0,r1=cur_rs.v1;int r2=cur_rs.v2,dest=cur_rs.dest;
                switch (cur_rs.type)
                {
                    case OP_LB:
                        val=(signed char)memory[r1+signed12(r2)];
                        break;
                    case OP_LBU:
                        val=memory[r1+signed12(r2)];
                        break;
                    case OP_LH:
                        val=(signed short)((unsigned short)memory[r1+signed12(r2)]|((unsigned short)memory[r1+signed12(r2)+1]<<8));
                        break;
                    case OP_LHU:
                        val=((unsigned short)memory[r1+signed12(r2)]|((unsigned short)memory[r1+signed12(r2)+1]<<8));
                        break;
                    case OP_LW:
                        val=(((unsigned int)memory[r1+signed12(r2)])|((unsigned int)memory[r1+signed12(r2)+1]<<8)|((unsigned int)memory[r1+signed12(r2)+2]<<16)|((unsigned int)memory[r1+signed12(r2)+3]<<24));
                        break;
                    case OP_SB:
                    case OP_SH:
                    case OP_SW:
                        val=r2;
                        dest=r1+signed12(dest);
                        break;
                }
                return CDB(cur_rs.id,val,dest);
            }
            return CDB();
        }   

};


class BRU: public Executer{
    public:
         CDB execute()//return value is CDB broadcast
         {
           if (run())
           {
                bool val=0;
                unsigned int r1=cur_rs.v1,r2=cur_rs.v2,dest=cur_rs.dest;
                switch (cur_rs.type)
                {
                    case OP_BEQ:
                        val=(r1==r2)^dest;
                        break;
                    case OP_BGE:
                        val=((signed int)r1>=(signed int)r2)^dest;
                        break;
                    case OP_BGEU:
                        val=(r1>=r2)^dest;
                        break;
                    case OP_BLT:
                        val=((signed int)r1<(signed int)r2)^dest;
                        break;
                    case OP_BLTU:
                        val=(r1<r2)^dest;
                        break;
                    case OP_BNE:
                        val=(r1!=r2)^dest;
                        break;
                }
                return CDB(cur_rs.id,val,-2);
            }
           return CDB();
        }
};