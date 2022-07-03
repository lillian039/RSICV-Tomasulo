#include <iostream>
#include <cstdio>
#include "Tomasulo.hpp"
#include "RAM.hpp"
using namespace std;


int main() {
 //   freopen("queens.data", "r", stdin);
 //   freopen("me.out", "w", stdout);
    getCommand();
    reset();
    while (1) {
    //    printf("%04x ",PC);
        Clock++;
      // if(Clock>=4000)break;
        Commit();
        WriteResult();
        Execute();
        Issue();
    }
}
