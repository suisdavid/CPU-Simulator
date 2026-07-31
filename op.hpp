#pragma once
#include <iostream>
using namespace std;
enum OP_TYPE{
    OP_ADD,
    OP_SUB,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_SLL,
    OP_SRL,
    OP_SRA,
    OP_SLT,
    OP_SLTU,
    OP_ADDI,
    OP_ANDI,
    OP_ORI,
    OP_XORI,
    OP_SLLI,
    OP_SRLI,
    OP_SRAI,
    OP_SLTI,
    OP_SLTIU,
    OP_LB,
    OP_LBU,
    OP_LH,
    OP_LHU,
    OP_LW,
    OP_SB,
    OP_SH,
    OP_SW,
    OP_BEQ,
    OP_BGE,
    OP_BGEU,
    OP_BLT,
    OP_BLTU,
    OP_BNE,
    OP_JAL,
    OP_JALR,
    OP_AUIPC,
    OP_LUI,
    OP_NONE
};
struct op{
    OP_TYPE type;
    unsigned int rd,r1,r2;
    unsigned int id;
    op(OP_TYPE type_=OP_NONE,int rd_=0,int r1_=0,int r2_=0){
        type=type_;rd=rd_;r1=r1_;r2=r2_;
    }
};

op decode(unsigned int x){
    int opcode=(x&127);
    if (opcode==0b0110011){//R:basic arithmetics
        int funct3=((x>>12)&7),funct7=(x>>25),rd=((x>>7)&31),r1=((x>>15)&31),r2=((x>>20)&31);
        if (funct3==0b000){//add or sub
            if (funct7==0b0000000){//add
                return op(OP_ADD,rd,r1,r2);
            }
            else if (funct7==0b0100000){//sub
                return op(OP_SUB,rd,r1,r2);
            }
        }
        else if (funct3==0b111){//and
            return op(OP_AND,rd,r1,r2);
        }
        else if (funct3==0b110){//or
            return op(OP_OR,rd,r1,r2);
        }
        else if (funct3==0b100){//xor
            return op(OP_XOR,rd,r1,r2);
        }
        else if (funct3==0b001){//sll
            return op(OP_SLL,rd,r1,r2);
        }
        else if (funct3==0b101){//srl or sra
            if (funct7==0b0000000){//srl
                return op(OP_SRL,rd,r1,r2);
            }
            else if (funct7==0b0100000){//sra
                return op(OP_SRA,rd,r1,r2);
            }
        }
        else if (funct3==0b010){//slt
            return op(OP_SLT,rd,r1,r2);
        }
        else if (funct3==0b011){
            return op(OP_SLTU,rd,r1,r2);
        }
    }
    else if (opcode==0b0010011){//I(*):arithmetics
        int funct3=((x>>12)&7),rd=((x>>7)&31),r1=((x>>15)&31),imm=(x>>20);
        if (funct3==0b000){//addi
            return op(OP_ADDI,rd,r1,imm);
        }
        else if (funct3==0b111){//andi
            return op(OP_ANDI,rd,r1,imm);
        }
        else if (funct3==0b110){//ori
            return op(OP_ORI,rd,r1,imm);
        }
        else if (funct3==0b100){//xori
            return op(OP_XORI,rd,r1,imm);
        }
        else if (funct3==0b001){//slli
            return op(OP_SLLI,rd,r1,imm);
        }
        else if (funct3==0b101){//srli or srai
            int funct7=(x>>25),imm=((x>>20)&31);
            if (funct7==0b0000000){//srli
                return op(OP_SRLI,rd,r1,imm);
            }
            else if (funct7==0b0100000){//srai
                return op(OP_SRAI,rd,r1,imm);
            }
        }
        else if (funct3==0b010){//slti
            return op(OP_SLTI,rd,r1,imm);
        }
        else if (funct3==0b011){//sltiu
            return op(OP_SLTIU,rd,r1,imm);
        }
    }
    else if (opcode==0b0000011){//I:memory
        int funct3=((x>>12)&7),rd=((x>>7)&31),r1=((x>>15)&31),imm=(x>>20);
        if (funct3==0b000){//lb
            return op(OP_LB,rd,r1,imm);
        }
        else if (funct3==0b100){//lbu
            return op(OP_LBU,rd,r1,imm);
        }
        else if (funct3==0b001){//lh
            return op(OP_LH,rd,r1,imm);
        }
        else if (funct3==0b101){//lhu
            return op(OP_LHU,rd,r1,imm);
        }
        else if (funct3==0b010){//lw
            return op(OP_LW,rd,r1,imm);
        }
    }
    else if (opcode==0b0100011){//S:memory
        int funct3=((x>>12)&7),funct7=(x>>25),r1=((x>>15)&31),r2=((x>>20)&31),imm=(((x>>25)<<5)|((x>>7)&31));
        if (funct3==0b000){//sb
            return op(OP_SB,imm,r1,r2);
        }
        else if (funct3==0b001){//sh
            return op(OP_SH,imm,r1,r2);
        }
        else if (funct3==0b010){//sw
            return op(OP_SW,imm,r1,r2);
        }
    }
    else if (opcode==0b1100011){//B:control
        int funct3=((x>>12)&7),funct7=(x>>25),r1=((x>>15)&31),r2=((x>>20)&31),imm=(((x>>31)<<12)|(((x>>7)&1)<<11)|(((x>>25)&63)<<5)|(((x>>8)&15)<<1));
        if (funct3==0b000){//beq
            return op(OP_BEQ,imm,r1,r2);
        }
        else if (funct3==0b101){//bge
            return op(OP_BGE,imm,r1,r2);
        }
        else if (funct3==0b111){//bgeu
            return op(OP_BGEU,imm,r1,r2);
        }
        else if (funct3==0b100){//blt
            return op(OP_BLT,imm,r1,r2);
        }
        else if (funct3==0b110){//bltu
            return op(OP_BLTU,imm,r1,r2);
        }
        else if (funct3==0b001){//bne
            return op(OP_BNE,imm,r1,r2);
        }
    }
    else if (opcode==0b1101111){//J:control
        int rd=((x>>7)&31),imm=(((x>>31)<<20)|(((x>>12)&255)<<12)|(((x>>20)&1)<<11)|(((x>>21)&1023)<<1));
        return op(OP_JAL,rd,imm,0);
    }
    else if (opcode==0b1100111){//I:control
        int funct3=((x>>12)&7),rd=((x>>7)&31),r1=((x>>15)&31),imm=(x>>20);
        return op(OP_JALR,rd,r1,imm);
    }
    else if (opcode==0b0010111){//U:auipc
        int rd=((x>>7)&31),imm=((x>>12)<<12);
        return op(OP_AUIPC,rd,imm,0);
    }
    else if (opcode==0b0110111){//U:lui
        int rd=((x>>7)&31),imm=((x>>12)<<12);
        return op(OP_LUI,rd,imm,0);
    }
    return op();
}
