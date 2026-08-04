#pragma once
const int BIT=8;
const int SIZE=(1<<BIT);
const int HISTORY_MASK=(1<<BIT)-1;
const int KBIT=4;
const int KSIZE=(1<<(BIT+KBIT));
class Predictor{//mixed predictor
    private:
        unsigned char PHT[KSIZE];//2 bit saturate predictor,used for local prediction
        unsigned char LHT[SIZE];//k-bit LOCAL HISTORY
        unsigned char Gshare_status[SIZE];//used for global prediction,00:strongly not taken, 01: weakly not taken, 02: weakly taken, 03: strongly taken
        unsigned char score[SIZE];//if score>=2,use Gshare, otherwise use BHT
        int history;//BIT bits of history predicted results
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
            for (int i=0;i<SIZE;i++){LHT[i]=0;Gshare_status[i]=1;score[i]=1;}
            for (int i=0;i<KSIZE;i++){
                PHT[i]=1;
            }
            history=0;
            has_buffer=0;
        }
        int query(int addr){
            int subaddr=((addr>>2)&(SIZE-1)),gshare_res=(Gshare_status[subaddr^history]>=2),pht_res=(PHT[(subaddr<<KBIT)|LHT[subaddr]]>=2);
            int res=(score[subaddr]>=2?gshare_res:pht_res),used=(score[subaddr]>=2),different_result=(gshare_res!=pht_res),globalhistory=history,localhistory=LHT[subaddr];
            return (localhistory<<(3+BIT))|(globalhistory<<3)|(different_result<<2)|(used<<1)|res;
        }
        void add(int addr,int result){
            buffer_addr=addr;buffer_result=result;has_buffer=1;
        }
        void update(){
            if (!has_buffer){return;}
            has_buffer=0;
            int subaddr=((buffer_addr>>2)&(SIZE-1)),localhistory=((buffer_result>>(4+BIT))&(KSIZE-1)),globalhistory=((buffer_result>>4)&HISTORY_MASK),reverted=(buffer_result&1),taken=(((buffer_result)>>1)&1),used=(((buffer_result)>>2)&1),different_result=(((buffer_result)>>3)&1);
            if (reverted){
                if (different_result){
                    score[subaddr]=trans(score[subaddr],(used==1?-1:1));
                }
            }
            else{
                if (different_result)
                {
                    score[subaddr]=trans(score[subaddr],(used==1?1:-1));
                }
            }
            Gshare_status[subaddr^globalhistory]=trans(Gshare_status[subaddr^globalhistory],(taken^reverted)?1:-1);
            PHT[(subaddr<<KBIT)|localhistory]=trans(PHT[(subaddr<<KBIT)|localhistory],(taken^reverted)?1:-1);
            history=(((globalhistory<<1)|(taken^reverted))&HISTORY_MASK);
            LHT[subaddr]=(((localhistory<<1)|(taken^reverted))&((1<<KBIT)-1));//update history only after the branch result committed
        }
};