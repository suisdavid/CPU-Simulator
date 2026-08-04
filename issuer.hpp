#pragma once
#include "reg.hpp"
#include "op.hpp"
#include "RS.hpp"
#include "executer.hpp"
#include "committer.hpp"
extern ALU alu;
extern LSU lsu;
extern BRU bru;
extern Committer committer;
extern unsigned char memory[];
extern Register reg[];

class Issuer{
    public:
    int id,nex_want;
    op cur_op;
    Issuer(){
        id=-1;nex_want=0;
    }
    int issue(int clk)
    {
        if (id==-1||id!=nex_want){return nex_want;}
      //  cout<<"Issue: "<<id<<" "<<clk<<endl;
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
                if (alu.full()||committer.full()){return nex_want;}
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
                nex_want+=4;
                return nex_want;
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
                if (alu.full()||committer.full()){return nex_want;}
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
                nex_want+=4;
                return nex_want;
            case OP_LB:
            case OP_LBU:
            case OP_LH:
            case OP_LHU:
            case OP_LW:
                //load
                if (lsu.full()||committer.full()){return nex_want;}
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
                nex_want+=4;
                return nex_want;
            case OP_SB:
            case OP_SH:
            case OP_SW:
                //store
                if (lsu.full()||committer.full()){return nex_want;}
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
                nex_want+=4;
                return nex_want;
            case OP_BEQ:
            case OP_BGE:
            case OP_BGEU:
            case OP_BLT:
            case OP_BLTU:
            case OP_BNE:
                //branch
                if (bru.full()||committer.full()){return nex_want;}
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
                nex_want+=4;
                return nex_want;
            case OP_JAL:
                if (alu.full()||committer.full()){return nex_want;}
                rs.v1=id+4;
                rs.dest=rd;
                reg[rd].prod=clk;
                alu.add(rs);
                committer.add(ROB(clk,rd));
                nex_want+=signed21(r1);
                return nex_want;
            case OP_JALR:
                if (alu.full()||committer.full()){return nex_want;}
                if (rd==r1){
                    rs.v1=id+4;
                    rs.dest=rd;
                    reg[rd].prod=clk;
                    alu.add(rs);
                    committer.add(ROB(clk,rd));
                    nex_want+=4+signed12(r2);
                    return nex_want;
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
                        return nex_want;
                    }
                }
                rs.v1=id+4;
                rs.dest=rd;
                reg[rd].prod=clk;
                alu.add(rs);
                committer.add(ROB(clk,rd));
                nex_want+=val+signed12(r2)-id;
                return nex_want;
            case OP_AUIPC:
                if (alu.full()||committer.full()){return nex_want;}
                rs.v1=id+r1;
                rs.dest=rd;
                reg[rd].prod=clk;
                alu.add(rs);
                committer.add(ROB(clk,rd));
                nex_want+=4;
                return nex_want;
            case OP_LUI:
                if (alu.full()||committer.full()){return nex_want;}
                rs.v1=r1;
                rs.dest=rd;
                reg[rd].prod=clk;
                alu.add(rs);
                committer.add(ROB(clk,rd));
                nex_want+=4;
                return nex_want;
            default:
                return nex_want;
        }
    }
};
