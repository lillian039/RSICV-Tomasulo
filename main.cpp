#include <iostream>
#include <cstdio>
#include "Tomasulo.hpp"
#include "RAM.hpp"
using namespace std;


int main() {
 //   freopen("expr.data", "r", stdin);
 //   freopen("me.out", "w", stdout);
    getCommand();
    reset();
    while (1) {
     //   printf("%04x ",PC);
        Clock++;
        Commit();
        WriteResult();
        Execute();
        Issue();
    }
}
