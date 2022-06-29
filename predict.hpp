#ifndef TESTCASES_FOR_RISCV_PREDICT_HPP
#define TESTCASES_FOR_RISCV_PREDICT_HPP
struct Branch_Target_Buffer{
    int type;
    unsigned PC;
    int tbb[2];//two_bit_bimodal 二位饱和计数器
}BTB[1000000];
#endif //TESTCASES_FOR_RISCV_PREDICT_HPP
