#pragma once
#include <iostream>
const int maxn=0x40000;
const int block_num=(1<<8);
const int block_size=maxn/(4*block_num);
const int cache_size=(1<<4);
using namespace std;

typedef pair<int,unsigned char> Entry;
int hex2int(unsigned char c){
    if (c>='0'&&c<='9'){
        return c-'0';
    }
    if (c>='a'&&c<='f'){
        return c-'a'+10;
    }
    return c-'A'+10;
}
class Memory{
    private:
        unsigned char RAM[maxn];
        Entry cache[4][block_num][cache_size];//(addr,value(1 byte)) pair
        unsigned char cache_cnt[4][block_num];//four sets of caches, for addr mod 4 =0,1,2,3 respectively,m
        pair<bool,Entry>updateEntries[4];//for updating
    public:
        Memory(){
            for (int j=0;j<4;j++)
            {
                for (int i=0;i<block_num;i++){
                    cache_cnt[j][i]=0;
                }
            }
        }
        void init()
        {
            int id=0;unsigned char c=getchar();
            while (c!=255)
            {
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
                    RAM[id++]=val;
                }
                c=getchar();
            }
        }

        unsigned int fetch(int id)
        {
            return ((unsigned int)RAM[id])|((unsigned int)RAM[id+1]<<8)|((unsigned int)RAM[id+2]<<16)|((unsigned int)RAM[id+3]<<24);
        }
        pair<bool,unsigned char>load(int addr){//bool: whether cache hit or not
            int set_id=(addr&3),block_id=(addr>>2)/block_size;
            for (int i=0;i<cache_cnt[set_id][block_id];i++){
                if (cache[set_id][block_id][i].first==addr){
                    Entry entry=cache[set_id][block_id][i];
                    updateEntries[set_id].first=1;
                    updateEntries[set_id].second=entry;
                    return make_pair(1,entry.second);
                }
            }
            //Not Found, update cache
            Entry entry=make_pair(addr,RAM[addr]);
            updateEntries[set_id].first=1;
            updateEntries[set_id].second=entry;
            return make_pair(0,entry.second);
        }
        bool store(unsigned char value,int addr){
            int set_id=(addr&3),block_id=(addr>>2)/block_size;
            for (int i=0;i<cache_cnt[set_id][block_id];i++){
                if (cache[set_id][block_id][i].first==addr){
                    Entry entry=cache[set_id][block_id][i];entry.second=value;
                    if (!updateEntries[set_id].first){
                        updateEntries[set_id].first=1;
                        updateEntries[set_id].second=entry;
                    }
                    return 1;
                }
            }
            //Not Found, update cache
            RAM[addr]=value;
            if (!updateEntries[set_id].first)
            {
                updateEntries[set_id].first=1;
                updateEntries[set_id].second=make_pair(addr,value);
            }
            return 0;
        }  

        void update(){
            for (int set_id=0;set_id<4;set_id++){
                if (!updateEntries[set_id].first){continue;}
                updateEntries[set_id].first=0;
                Entry entry=updateEntries[set_id].second;
                int addr=entry.first,block_id=(addr>>2)/block_size,flag=0;
                for (int i=0;i<cache_cnt[set_id][block_id];i++)//update in cache
                {
                    if (cache[set_id][block_id][i].first==addr){
                        for (int j=i;j>0;j--){
                            cache[set_id][block_id][j]=cache[set_id][block_id][j-1];
                        }
                        cache[set_id][block_id][0]=entry;
                        flag=1;break;
                    }
                }
                if (flag){continue;}
                //add to cache
                if (cache_cnt[set_id][block_id]==cache_size){//remove the oldest entry,write back to memory
                    RAM[cache[set_id][block_id][cache_size-1].first]=cache[set_id][block_id][cache_size-1].second;
                    cache_cnt[set_id][block_id]--;
                }
                for (int i=cache_cnt[set_id][block_id];i>0;i--)
                {
                    cache[set_id][block_id][i]=cache[set_id][block_id][i-1];
                }
                cache[set_id][block_id][0]=entry;cache_cnt[set_id][block_id]++;
            }
        }

};
