#include <iostream>
#include "reg.hpp"
#include "op.hpp"
#include "main_naive.hpp"//for debugging
using namespace std;

const int maxn=0x40000;
unsigned char memory[maxn];
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

int main(){
    read_op();
    for (int i=0;i<maxn;i++){
        naive::_memory[i]=memory[i];
    }
    int clk=0,a=0;
    while (!a){
        clk++;
        a=naive::do_naive();
    }
    cout<<a<<" "<<clk<<endl;
    return 0;
}