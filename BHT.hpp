#pragma once
const int BIT=12;
const int SIZE=(1<<BIT);
class BHT{
    private:
        unsigned char status[SIZE];//00:strongly not taken, 01: weakly not taken, 02: weakly taken, 03: strongly not taken
    public:
        BHT(){
            for (int i=0;i<SIZE;i++){status[i]=0;}
        }
        bool query(int addr){
            int subaddr=((addr>>2)&(SIZE-1));
            return status[subaddr]>=2;
        }
        void add(int addr,int result){
            int subaddr=((addr>>2)&(SIZE-1));
            if (result){
                status[subaddr]=(status[subaddr]==3?3:status[subaddr]+1);
            }
            else{
                status[subaddr]=(status[subaddr]==0?0:status[subaddr]-1);
            }
        }
}bht;