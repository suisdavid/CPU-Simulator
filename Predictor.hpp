#pragma once
const int BIT=12;
const int SIZE=(1<<BIT);
class Predictor{//mixed predictor
    private:
        unsigned char BHT_status[SIZE];//00:strongly not taken, 01: weakly not taken, 02: weakly taken, 03: strongly not taken
        unsigned char Gshare_status[SIZE];
        int history;//BIT bits of history predicted results
        int score;//if score>=2,use Gshare, otherwise use BHT
        int buffer_addr,buffer_result;
        bool has_buffer;
        int trans(int x,int y){
            x+=y;
            if (x>3){return 3;}
            if (x<0){return 0;}
            return x;
        }
    public:
        Predictor(){
            for (int i=0;i<SIZE;i++){BHT_status[i]=Gshare_status[i]=0;}
            history=0;score=0;
            has_buffer=0;
        }
        int query(int addr){
            int subaddr=((addr>>2)&(SIZE-1)),res=0;
            if (score>=2)//use Gshare
            {
                res=(Gshare_status[subaddr^history]>=2);
            }
            else{
                res=(BHT_status[subaddr]>=2);
            }
            int used=(score>=2),different_result=((Gshare_status[subaddr^history]>=2)!=(BHT_status[subaddr]>=2)),pre_history=history;
            history=(((history<<1)|res)&(SIZE-1));
            return pre_history*8+different_result*4+used*2+res;
        }
        void add(int addr,int result){
            buffer_addr=addr;buffer_result=result;has_buffer=1;
        }
        void update(){
            if (!has_buffer){return;}
            has_buffer=0;
            int subaddr=((buffer_addr>>2)&(SIZE-1)),pre_history=(buffer_result>>4),reverted=(buffer_result&1),taken=(((buffer_result)>>1)&1),used=(((buffer_result)>>2)&1),different_result=(((buffer_result)>>3)&1);
            if (reverted){
                if (different_result){
                    score=trans(score,(used==1?-1:1));
                }
            }
            else{
                if (different_result)
                {
                    score=trans(score,(used==1?1:-1));
                }
            }
            if (used)
            {
                Gshare_status[subaddr^pre_history]=trans(Gshare_status[subaddr^pre_history],(taken^reverted)?1:-1);
            }
            else
            {
                BHT_status[subaddr]=trans(BHT_status[subaddr],(taken^reverted)?1:-1);
            }
        }
};