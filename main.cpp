#include <iostream>
#include <fstream>
#include "reg.hpp"
#include "op.hpp"
using namespace std;

const int maxm=0x40000;
unsigned char memory[maxm];
Register reg[32],pc;

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

unsigned int get_op(int id){
    return ((unsigned int)memory[id])|((unsigned int)memory[id+1]<<8)|((unsigned int)memory[id+2]<<16)|((unsigned int)memory[id+3]<<24);
}
int main(){
    read_op();
    while (1){
        reg[x0].val=0;
        unsigned int x=get_op(pc.val);
        if (x==0x0ff00513){
            cout<<(reg[a0].val&255)<<endl;//return 
            break;
        }
        //cout<<pc.val<<" "<<x<<endl;
        op cur_op=parse_op(x);
        OP_TYPE type=cur_op.type;int rd=cur_op.rd,r1=cur_op.r1,r2=cur_op.r2;
       // cout<<pc.val<<" "<<type<<" "<<rd<<" "<<r1<<" "<<r2<<" "<<reg[a0].val<<endl;
        switch (type){
            case OP_ADD:
                reg[rd].val=reg[r1].val+reg[r2].val;
                break;
            case OP_SUB:
                reg[rd].val=reg[r1].val-reg[r2].val;
                break;
            case OP_AND:
                reg[rd].val=(reg[r1].val&reg[r2].val);
                break;
            case OP_OR:
                reg[rd].val=(reg[r1].val|reg[r2].val);
                break;
            case OP_XOR:
                reg[rd].val=(reg[r1].val^reg[r2].val);
                break;
            case OP_SLL:
                reg[rd].val=(reg[r1].val<<reg[r2].val);
                break;
            case OP_SRL:
                reg[rd].val=(reg[r1].val>>reg[r2].val);
                break;
            case OP_SRA:
                reg[rd].val=((signed int)reg[r1].val>>reg[r2].val);
                break;
            case OP_SLT:
                reg[rd].val=((signed int)reg[r1].val<(signed int)reg[r2].val?1:0);
                break;
            case OP_SLTU:
                reg[rd].val=(reg[r1].val<reg[r2].val?1:0);
                break;
            case OP_ADDI:
                reg[rd].val=reg[r1].val+signed12(r2);
                break;
            case OP_ANDI:
                reg[rd].val=(reg[r1].val&signed12(r2));
                break;
            case OP_ORI:
                reg[rd].val=(reg[r1].val|signed12(r2));
                break;
            case OP_XORI:
                reg[rd].val=(reg[r1].val^signed12(r2));
                break;
            case OP_SLLI:
                reg[rd].val=(reg[r1].val<<r2);
                break;
            case OP_SRLI:
                reg[rd].val=(reg[r1].val>>r2);
                break;
            case OP_SRAI:
                reg[rd].val=((signed int)reg[r1].val>>r2);
                break;
            case OP_SLTI:
                reg[rd].val=((signed int)reg[r1].val<signed12(r2)?1:0);
                break;
            case OP_SLTIU:
                reg[rd].val=(reg[r1].val<(unsigned int)signed12(r2)?1:0);
                break;
            case OP_LB:
                reg[rd].val=(signed char)memory[reg[r1].val+signed12(r2)];
                break;
            case OP_LBU:
                reg[rd].val=memory[reg[r1].val+signed12(r2)];
                break;
            case OP_LH:
                reg[rd].val=(signed short)((unsigned short)memory[reg[r1].val+signed12(r2)]|((unsigned short)memory[reg[r1].val+signed12(r2)+1]<<8));
                break;
            case OP_LHU:
                reg[rd].val=((unsigned short)memory[reg[r1].val+signed12(r2)]|((unsigned short)memory[reg[r1].val+signed12(r2)+1]<<8));
                break;
            case OP_LW:
                reg[rd].val=get_op(reg[r1].val+signed12(r2));
                break;
            case OP_SB:
                memory[reg[r1].val+signed12(r2)]=(reg[rd].val&255);
                break;
            case OP_SH:
                memory[reg[r1].val+signed12(r2)]=(reg[rd].val&255);
                memory[reg[r1].val+signed12(r2)+1]=((reg[rd].val>>8)&255);
                break;
            case OP_SW:
                memory[reg[r1].val+signed12(r2)]=(reg[rd].val&255);
                memory[reg[r1].val+signed12(r2)+1]=((reg[rd].val>>8)&255);
                memory[reg[r1].val+signed12(r2)+2]=((reg[rd].val>>16)&255);
                memory[reg[r1].val+signed12(r2)+3]=((reg[rd].val>>24)&255);
                break;
            case OP_BEQ:
                if (reg[r1].val==reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_BGE:
                if ((signed int)reg[r1].val>=(signed int)reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_BGEU:
                if (reg[r1].val>=reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_BLT:
                if ((signed int)reg[r1].val<(signed int)reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_BLTU:
                if (reg[r1].val<reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_BNE:
                if (reg[r1].val!=reg[r2].val){
                    pc.val+=signed13(rd)-4;
                }
                break;
            case OP_JAL:
                reg[rd].val=pc.val+4;
                pc.val+=signed21(r1)-4;
                break;
            case OP_JALR:
                reg[rd].val=pc.val+4;
                pc.val=reg[r1].val+signed12(r2)-4;
                break;
            case OP_AUIPC:
                reg[rd].val=pc.val+r1;
                break;
            case OP_LUI:
                reg[rd].val=r1;
                break;
        }
        pc.val+=4;
    }
    return 0;
}