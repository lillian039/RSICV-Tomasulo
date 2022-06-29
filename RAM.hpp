#ifndef TESTCASES_FOR_RISCV_RAM_HPP
#define TESTCASES_FOR_RISCV_RAM_HPP

unsigned int mem[1000000], PC = 0, reg[32],des=0;
unsigned int Clock;

unsigned int me_to_int(std::string num) {
    unsigned int d = 0;
    int power[8] = {28, 24, 20, 16, 12, 8, 4, 0};
    for (int i = 0; i <= 7; i++) {
        if ('0' <= num[i] && num[i] <= '9')d += (num[i] - '0') * (1 << power[i]);
        else d += (num[i] - 'A' + 10) * (1 << power[i]);
    }
    return d;
}

void set_memory(char first, int des) {
    int power[8] = {4, 0, 4, 0, 4, 0, 4, 0};
    for (int i = 0; i < 8; i++) {
        if ('0' <= first && first <= '9')mem[des + i / 2] += (first - '0') * (1 << power[i]);
        else mem[des + i / 2] += (first - 'A' + 10) * (1 << power[i]);
        if (i <= 6)std::cin >> first;
    }
}

void getCommand() {
    std::string num;
    char first_char;
    while (std::cin >> first_char) {
        if (first_char == '@') {
            std::cin >> num;
            des = me_to_int(num);
        } else {
            set_memory(first_char, des);
            des += 4;
        }
    }
}

unsigned int getOrder(){
    return ((mem[PC] & ((1 << 9) - 1)) + ((mem[PC + 1] & ((1 << 17) - 1)) << 8) +
            ((mem[PC + 2] & ((1 << 25) - 1)) << 16) + (mem[PC + 3] << 24));
}

#endif //TESTCASES_FOR_RISCV_RAM_HPP
