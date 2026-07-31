#include <iostream>
#include "reg.hpp"
#include "op.hpp"
#include "RS.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "committer.hpp"
#include "executer.hpp"
#include "main_naive.hpp"//for debugging
using namespace std;

const int maxn=0x40000;
unsigned char memory[maxn];
Register reg[32],pc;
ALU alu;
LSU lsu;
BRU bru;
Committer committer;
bool halt;
int hex2int(unsigned char c){
    if (c>='0'&&c<='9'){
        return c-'0';
    }
    if (c>='a'&&c<='f'){
        return c-'a'+10;
    }
    return c-'A'+10;
}
void read_op(){
    int id=0;unsigned char c=getchar();
    while (c!=255){
        if (c=='@'){
            id=0;
            for (int i=0;i<8;i++){
                c=getchar();
                id=id*16+hex2int(c);
            }
        }
        else if ((c>='0'&&c<='9')||(c>='A'&&c<='F')){
            int val=hex2int(c);
            c=getchar();
            val=val*16+hex2int(c);
            memory[id++]=val;
        }
        c=getchar();
    }
}

int issue(int clk,int id,op cur_op){
    if (id==-1){return 4;}
   // cout<<"Issue: "<<id<<" "<<clk<<endl;
    OP_TYPE type=cur_op.type;int rd=cur_op.rd,r1=cur_op.r1,r2=cur_op.r2;
    RS rs;rs.type=type;rs.id=clk;//label
    unsigned int val=0;
    switch (type){
        case OP_ADD:
        case OP_SUB:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_SLL:
        case OP_SRL:
        case OP_SRA:
        case OP_SLT:
        case OP_SLTU:
            //alu(no immediate)
            if (alu.full()||committer.full()){return 0;}
            if (reg[r1].prod==-1){
                rs.v1=reg[r1].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    rs.v1=temp.first;
                }
                else{
                    rs.q1=reg[r1].prod;
                }
            }
            if (reg[r2].prod==-1){
                rs.v2=reg[r2].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r2].prod);
                if (temp.second){
                    rs.v2=temp.first;
                }
                else{
                    rs.q2=reg[r2].prod;
                }
            }
            rs.dest=rd;
            reg[rd].prod=clk;
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return 4;
        case OP_ADDI:
        case OP_ANDI:
        case OP_ORI:
        case OP_XORI:
        case OP_SLLI:
        case OP_SRLI:
        case OP_SRAI:
        case OP_SLTI:
        case OP_SLTIU:
            //alu(with immediate)
            if (alu.full()||committer.full()){return 0;}
            if (reg[r1].prod==-1){
                rs.v1=reg[r1].val;
            }
            else{
                 pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    rs.v1=temp.first;
                }
                else{
                    rs.q1=reg[r1].prod;
                }
            }
            rs.v2=r2;
            rs.dest=rd;
            reg[rd].prod=clk;
            if (type==OP_ADDI&&r1==0&&r2==255&&rd==10){//halt instruction
                rs.is_halt=true;
            }
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return 4;
        case OP_LB:
        case OP_LBU:
        case OP_LH:
        case OP_LHU:
        case OP_LW:
            //load
            if (lsu.full()||committer.full()){return 0;}
            if (reg[r1].prod==-1){
                rs.v1=reg[r1].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    rs.v1=temp.first;
                }
                else{
                    rs.q1=reg[r1].prod;
                }
            }
            rs.v2=r2;
            rs.dest=rd;
            reg[rd].prod=clk;
            lsu.add(rs);
            committer.add(ROB(clk,rd));
            return 4;
        case OP_SB:
        case OP_SH:
        case OP_SW:
            //store
            if (lsu.full()||committer.full()){return 0;}
            if (reg[r1].prod==-1){
                rs.v1=reg[r1].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    rs.v1=temp.first;
                }
                else{
                    rs.q1=reg[r1].prod;
                }
            }
            if (reg[r2].prod==-1){
                rs.v2=reg[r2].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r2].prod);
                if (temp.second){
                    rs.v2=temp.first;
                }
                else{
                    rs.q2=reg[r2].prod;
                }
            }
            rs.dest=rd;
            lsu.add(rs);
            committer.add(ROB(clk,0,type==OP_SB?1:(type==OP_SH?2:4)));//change memory;
            return 4;
        case OP_BEQ:
        case OP_BGE:
        case OP_BGEU:
        case OP_BLT:
        case OP_BLTU:
        case OP_BNE:
            //branch
            if (bru.full()||committer.full()){return 0;}
            if (reg[r1].prod==-1){
                rs.v1=reg[r1].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    rs.v1=temp.first;
                }
                else{
                    rs.q1=reg[r1].prod;
                }
            }
            if (reg[r2].prod==-1){
                rs.v2=reg[r2].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r2].prod);
                if (temp.second){
                    rs.v2=temp.first;
                }
                else{
                    rs.q2=reg[r2].prod;
                }
            }
            rs.dest=0;//dest means whether choose to jump
            bru.add(rs);
            committer.add(ROB(clk,id+signed13(rd),-1));//dest is the unchosen pc value;
            return 4;
        case OP_JAL:
            if (alu.full()||committer.full()){return 0;}
            rs.v1=id+4;
            rs.dest=rd;
            reg[rd].prod=clk;
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return signed21(r1);
        case OP_JALR:
            if (alu.full()||committer.full()){return 0;}
            if (rd==r1){
                rs.v1=id+4;
                rs.dest=rd;
                reg[rd].prod=clk;
                alu.add(rs);
                committer.add(ROB(clk,rd));
                return id+4+signed21(r2);
            }
            if (reg[r1].prod==-1){
                val=reg[r1].val;
            }
            else{
                pair<unsigned int,bool>temp=committer.read(reg[r1].prod);
                if (temp.second){
                    val=temp.first;
                }
                else{
                    return 0;
                }
            }
            rs.v1=id+4;
            rs.dest=rd;
            reg[rd].prod=clk;
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return val+signed12(r2)-id;
        case OP_AUIPC:
            if (alu.full()||committer.full()){return 0;}
            rs.v1=id+r1;
            rs.dest=rd;
            reg[rd].prod=clk;
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return 4;
        case OP_LUI:
            if (alu.full()||committer.full()){return 0;}
            rs.v1=r1;
            rs.dest=rd;
            reg[rd].prod=clk;
            alu.add(rs);
            committer.add(ROB(clk,rd));
            return 4;
        default:
            return 0;
    }
}

void broadcast(CDB cdb){
    if (cdb.id==-1){return;}
    alu.receive(cdb);
    lsu.receive(cdb);
    bru.receive(cdb);
    committer.receive(cdb);
}

unsigned int fetch(int id){
    return ((unsigned int)memory[id])|((unsigned int)memory[id+1]<<8)|((unsigned int)memory[id+2]<<16)|((unsigned int)memory[id+3]<<24);
}

int main(){
    read_op();
    /*for (int i=0;i<maxn;i++){
        naive::_memory[i]=memory[i];
    }*/
    int clk=0;
    while (!halt){
        clk++;reg[x0].val=0;reg[x0].prod=-1;
        unsigned int opcode=fetch(pc.val);
        op cur_op=decode(opcode);
        int offset=issue(clk,pc.val,cur_op);//offset=0时需等待后续元件，fetcher&decoder空转
        broadcast(alu.execute());
        broadcast(lsu.execute());
        broadcast(bru.execute());
        int res=committer.Commit();
        if (res>0&&lsu.lst_store==res){
            lsu.lst_store=-1;//store completed
        }
        if (res<0){
            alu.clear(-res);//clear all RS whose id>-res
            if (lsu.lst_store>-res){
                lsu.lst_store=-1;
            }
            lsu.clear(-res);
            bru.clear(-res);
            for (int i=0;i<32;i++){
                if (reg[i].prod>-res){
                    reg[i].prod=-1;
                }
            }
        }
        else
        {
            pc.val+=offset;
        }
    }
    return 0;
}