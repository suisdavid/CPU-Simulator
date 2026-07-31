#pragma once
#include <iostream>
#include <fstream>
#include "reg.hpp"
#include "op.hpp"

namespace naive{
const int _maxm=0x40000;
unsigned char _memory[_maxm];
Register _reg[32],_pc;

int _signed12(unsigned int x){
    if (x<2048){return (int)x;}
    return (int)x-4096;
}

int _signed13(unsigned int x){
    if (x<4096){return (int)x;}
    return (int)x-8192;
}

int _signed21(unsigned int x){
    if (x<1048576){return (int)x;}
    return (int)x-2097152;
}

unsigned int _get_op(int id){
    return ((unsigned int)_memory[id])|((unsigned int)_memory[id+1]<<8)|((unsigned int)_memory[id+2]<<16)|((unsigned int)_memory[id+3]<<24);
}
int do_naive(){
    _reg[x0].val=0;
    unsigned int x=_get_op(_pc.val);
    if (x==0x0ff00513){
        return _reg[a0].val&255;
    }
    op cur_op=decode(x);
    OP_TYPE type=cur_op.type;int rd=cur_op.rd,r1=cur_op.r1,r2=cur_op.r2;
    // cout<<_pc.val<<" "<<type<<" "<<rd<<" "<<r1<<" "<<r2<<" "<<_reg[a0].val<<endl;
    switch (type){
        case OP_ADD:
            _reg[rd].val=_reg[r1].val+_reg[r2].val;
            break;
        case OP_SUB:
            _reg[rd].val=_reg[r1].val-_reg[r2].val;
            break;
        case OP_AND:
            _reg[rd].val=(_reg[r1].val&_reg[r2].val);
            break;
        case OP_OR:
            _reg[rd].val=(_reg[r1].val|_reg[r2].val);
            break;
        case OP_XOR:
            _reg[rd].val=(_reg[r1].val^_reg[r2].val);
            break;
        case OP_SLL:
            _reg[rd].val=(_reg[r1].val<<_reg[r2].val);
            break;
        case OP_SRL:
            _reg[rd].val=(_reg[r1].val>>_reg[r2].val);
            break;
        case OP_SRA:
            _reg[rd].val=((signed int)_reg[r1].val>>_reg[r2].val);
            break;
        case OP_SLT:
            _reg[rd].val=((signed int)_reg[r1].val<(signed int)_reg[r2].val?1:0);
            break;
        case OP_SLTU:
            _reg[rd].val=(_reg[r1].val<_reg[r2].val?1:0);
            break;
        case OP_ADDI:
            _reg[rd].val=_reg[r1].val+_signed12(r2);
            break;
        case OP_ANDI:
            _reg[rd].val=(_reg[r1].val&_signed12(r2));
            break;
        case OP_ORI:
            _reg[rd].val=(_reg[r1].val|_signed12(r2));
            break;
        case OP_XORI:
            _reg[rd].val=(_reg[r1].val^_signed12(r2));
            break;
        case OP_SLLI:
            _reg[rd].val=(_reg[r1].val<<r2);
            break;
        case OP_SRLI:
            _reg[rd].val=(_reg[r1].val>>r2);
            break;
        case OP_SRAI:
            _reg[rd].val=((signed int)_reg[r1].val>>r2);
            break;
        case OP_SLTI:
            _reg[rd].val=((signed int)_reg[r1].val<_signed12(r2)?1:0);
            break;
        case OP_SLTIU:
            _reg[rd].val=(_reg[r1].val<(unsigned int)_signed12(r2)?1:0);
            break;
        case OP_LB:
            _reg[rd].val=(signed char)_memory[_reg[r1].val+_signed12(r2)];
            break;
        case OP_LBU:
            _reg[rd].val=_memory[_reg[r1].val+_signed12(r2)];
            break;
        case OP_LH:
            _reg[rd].val=(signed short)((unsigned short)_memory[_reg[r1].val+_signed12(r2)]|((unsigned short)_memory[_reg[r1].val+_signed12(r2)+1]<<8));
            break;
        case OP_LHU:
            _reg[rd].val=((unsigned short)_memory[_reg[r1].val+_signed12(r2)]|((unsigned short)_memory[_reg[r1].val+_signed12(r2)+1]<<8));
            break;
        case OP_LW:
            _reg[rd].val=_get_op(_reg[r1].val+_signed12(r2));
            break;
        case OP_SB:
            _memory[_reg[r1].val+_signed12(rd)]=(_reg[r2].val&255);
            break;
        case OP_SH:
            _memory[_reg[r1].val+_signed12(rd)]=(_reg[r2].val&255);
            _memory[_reg[r1].val+_signed12(rd)+1]=((_reg[r2].val>>8)&255);
            break;
        case OP_SW:
            _memory[_reg[r1].val+_signed12(rd)]=(_reg[r2].val&255);
            _memory[_reg[r1].val+_signed12(rd)+1]=((_reg[r2].val>>8)&255);
            _memory[_reg[r1].val+_signed12(rd)+2]=((_reg[r2].val>>16)&255);
            _memory[_reg[r1].val+_signed12(rd)+3]=((_reg[r2].val>>24)&255);
            break;
        case OP_BEQ:
            if (_reg[r1].val==_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_BGE:
            if ((signed int)_reg[r1].val>=(signed int)_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_BGEU:
            if (_reg[r1].val>=_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_BLT:
            if ((signed int)_reg[r1].val<(signed int)_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_BLTU:
            if (_reg[r1].val<_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_BNE:
            if (_reg[r1].val!=_reg[r2].val){
                _pc.val+=_signed13(rd)-4;
            }
            break;
        case OP_JAL:
            _reg[rd].val=_pc.val+4;
            _pc.val+=_signed21(r1)-4;
            break;
        case OP_JALR:
            _reg[rd].val=_pc.val+4;
            _pc.val=_reg[r1].val+_signed12(r2)-4;
            break;
        case OP_AUIPC:
            _reg[rd].val=_pc.val+r1;
            break;
        case OP_LUI:
            _reg[rd].val=r1;
            break;
    }
    _pc.val+=4;
    return 0;
}
};