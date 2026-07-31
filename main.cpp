#include <iostream>
#include "reg.hpp"
#include "op.hpp"
#include "RS.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "decoder.hpp"
#include "issuer.hpp"
#include "committer.hpp"
#include "executer.hpp"
#include "main_naive.hpp"//for debugging
using namespace std;

const int maxn=0x40000;
unsigned char memory[maxn];
Register reg[32],pc;
Decoder decoder;
Issuer issuer;
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
    int clk=0;
    while (!halt){
        clk++;reg[x0].val=0;reg[x0].prod=-1;//period initialize

        //fetch
        pair<int,unsigned int>fetcher_output=std::make_pair(pc.val,fetch(pc.val));
        //decode
        pair<int,op>decode_output=decoder.decode();
        //issue
        int offset=issuer.issue(clk);
        //write back
        broadcast(alu.execute());
        broadcast(lsu.execute());
        broadcast(bru.execute());
        //commit
        int res=committer.Commit();
        if (res>0&&lsu.lst_store==res){
            lsu.lst_store=-1;//store completed
        }
        if (res<0){//revert
            issuer.nex_want=pc.val;
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
        //update
        decoder.id=fetcher_output.first;decoder.opcode=fetcher_output.second;
        if (decode_output.first==issuer.nex_want)
        {  
            issuer.id=decode_output.first;issuer.cur_op=decode_output.second;
            if (res>=0)
            {
                pc.val+=offset;
            }
        }
        else{
            issuer.id=-1;//rejected legacy decoder output
            pc.val=issuer.nex_want;
        }
    }
    return 0;
}