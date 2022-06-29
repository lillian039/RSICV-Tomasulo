#include <iostream>
#include <cstdio>
#include "Tomasulo.hpp"
#include "RAM.hpp"
using namespace std;


int main() {
 //   freopen("array_test2.data", "r", stdin);
 //   freopen("me.out", "w", stdout);
    getCommand();
    reset();
    while (1) {
        Clock++;
        Commit();
        WriteResult();
        Execute();
        Issue();
    }
}
