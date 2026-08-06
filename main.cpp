#include "memory.hpp"
#include "reg.hpp"
#include "op.hpp"
#include "RS.hpp"
#include "ROB.hpp"
#include "CDB.hpp"
#include "Predictor.hpp"
#include "decoder.hpp"
#include "issuer.hpp"
#include "committer.hpp"
#include "executer.hpp"
using namespace std;

Memory memory;
Register reg[32],pc;//record the last unknown store
Decoder decoder;
Issuer issuer;
ALU alu;
LSU lsu;
BRU bru;
CDB alu_cdb,lsu_cdb,bru_cdb,pre_alu_cdb,pre_lsu_cdb,pre_bru_cdb;
Committer committer;
Predictor predictor;
bool halt;
int flush_val,newaddr;//for flushing

void broadcast(CDB cdb){
    if (cdb.id==-1){return;}
    alu.receive(cdb);
    lsu.receive(cdb);
    bru.receive(cdb);
    committer.receive(cdb);
}


int main(){
    memory.init();
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
        pair<int,unsigned int>fetcher_output=make_pair(pc.val,memory.fetch(pc.val));
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
        predictor.update();
        memory.update();
    }
   // cout<<"clock="<<clk<<endl;
   // cout<<"hit= "<<memory.hit<<" not hit= "<<memory.nothit<<" accuracy= "<<(double)memory.hit/(memory.hit+memory.nothit)<<endl;
   // cout<<"branch predictor accuracy="<<(double)committer.right/committer.total<<endl;
    return 0;
}