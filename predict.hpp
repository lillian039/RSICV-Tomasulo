#ifndef TESTCASES_FOR_RISCV_PREDICT_HPP
#define TESTCASES_FOR_RISCV_PREDICT_HPP

enum counter {
    stronglyNotTaken, weaklyNotTaken, weaklyTaken, stronglyTaken
};

class Branch_Target_Buffers {
private:
    const unsigned int prime = 337;
    struct Branch_Target_Buffer {
        int sc = weaklyTaken;//saturating counter 二位饱和计数器
    } BTB[4000];


    int getHash(const unsigned &pc) {
        return (pc * prime) % 4000;
    }

public:
    void changeState(bool result, const unsigned &pc) {
        int idx = getHash(pc);
        if (result && BTB[idx].sc != stronglyTaken)BTB[idx].sc++;
        else if (!result && BTB[idx].sc != stronglyNotTaken)BTB[idx].sc--;
    }

    bool jump(const unsigned &pc) {
        int idx = getHash(pc);
        if (BTB[idx].sc == weaklyTaken || BTB[idx].sc == stronglyTaken)return true;
        else return false;
    }
} Predicter;


struct InstructionQueue {
private:
    static const int len = 16;
    int top = 0, size = 0;
    bool stop = false;

    struct InstruQueue {
        unsigned pc = 0, order = 0;
    } isq[len];

    bool full() {
        if (size == len)return true;
        return false;
    }

public:
    bool available() {
        return size;
    }

    void reset(unsigned des) {
     //   puts("reset!");
        top = 0, size = 0, stop = false, PC = des;
    }

    void GetInstructMem() {
        if (full() || stop)return;
        int rear = (top + size) % len;
        size++;
        unsigned order = getOrder();
        isq[rear].order = order, isq[rear].pc = PC;
        if (getOpcode(order) == JALR || getOpcode(order) == JAL)stop = true;
        else if (getType(order) == B) {
            unsigned int des = getImm(order) + PC;
            if (Predicter.jump(PC))PC = des;
            else PC += 4;
        } else PC += 4;
    }

    //for JALR and JAL
    void reStart(unsigned int des) {
        PC = des;
        stop = false;
    }

    std::pair<unsigned, unsigned> getCommand() {
     //   printf("%04x\n", isq[top].pc);
        return std::make_pair(isq[top].order, isq[top].pc);
    }

    void pop() {
        top = (top + 1) % len;
        size--;
    }

} ISQ;

#endif //TESTCASES_FOR_RISCV_PREDICT_HPP
