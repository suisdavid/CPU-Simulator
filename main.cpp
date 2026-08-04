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
Register reg[32],pc;//record the last unknown store
Decoder decoder;
Issuer issuer;
ALU alu;
LSU lsu;
BRU bru;
CDB alu_cdb,lsu_cdb,bru_cdb,pre_alu_cdb,pre_lsu_cdb,pre_bru_cdb;
Committer committer;
bool halt;
int flush_val,newaddr;//for flushing
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
    /*for (int i=0;i<maxn;i++){
        naive::_memory[i]=memory[i];
    }*/
    int clk=0;
    while (!halt){
        clk++;reg[x0].val=0;reg[x0].prod=-1;//period initialize
        if (flush_val){//flushing
            issuer.nex_want=pc.val=newaddr;
            alu.clear(flush_val);//clear all RS whose id>-res
            lsu.clear(flush_val);
            bru.clear(flush_val);
            for (int i=0;i<32;i++){
                if (reg[i].prod>flush_val){
                    reg[i].prod=-1;
                }
            }
            committer.clear();
            decoder.id=-1;
            issuer.id=-1;
            flush_val=0;
            continue;
        }
        //fetch
        pair<int,unsigned int>fetcher_output=std::make_pair(pc.val,fetch(pc.val));
        //decode
        pair<int,op>decode_output=decoder.decode();
        //issue
        int new_pc_val=issuer.issue(clk);
        //execute
        CDB alu_cdb=alu.execute();
        CDB lsu_cdb=lsu.execute();
        CDB bru_cdb=bru.execute();
        //commit
        pair<int,int>committer_output=committer.Commit();
        int res=committer_output.first;
        if (res<0){//revert
            flush_val=-res;
            newaddr=committer_output.second;
        }
        //update
        decoder.id=fetcher_output.first;decoder.opcode=fetcher_output.second;
        issuer.id=decode_output.first;issuer.cur_op=decode_output.second;
        pc.val=new_pc_val;
        broadcast(alu_cdb);broadcast(lsu_cdb);broadcast(bru_cdb);
        broadcast(pre_alu_cdb);broadcast(pre_lsu_cdb);broadcast(pre_bru_cdb);
        pre_alu_cdb=alu_cdb;pre_lsu_cdb=lsu_cdb;pre_bru_cdb=bru_cdb;
        alu.update();lsu.update();bru.update();
        committer.update();
    }
   // cout<<"clock="<<clk<<endl;
   // cout<<"branch predictor accuracy="<<(double)committer.right/committer.total<<endl;
    return 0;
}