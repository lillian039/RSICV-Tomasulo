#include <iostream>
#include <cstdio>
#include "Tomasulo.hpp"
#include "RAM.hpp"
using namespace std;


int main() {
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
