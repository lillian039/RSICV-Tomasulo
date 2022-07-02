#ifndef TESTCASES_FOR_RISCV_TOMASULO_HPP
#define TESTCASES_FOR_RISCV_TOMASULO_HPP

#include "RAM.hpp"
#include "parser.hpp"
#include "command.hpp"
#include "predict.hpp"

struct Reorder_Buffers;
struct LS_Buffers;

enum state {
    waiting = 1,
    finished = 2,
    empty = 3,
    addressed = 4,
    loading = 5,
    waitingStore = 6,
    storing = 7,
    waitingBroadcast = 8
};

struct CommonDataBus {
    unsigned int entry, result, pc;
} CDB;

struct Register_Status {//Register Status for renaming
    bool Busy;
    int Reorder;
} RegisterStat[32];

struct Reorder_Buffer {
    int Entry;
    int Type;
    bool Ready;
    unsigned int Instruct, Des, Value, PC_now, PC_des;
};

struct Reorder_Buffers {
private:
    static const int len = 8;

    int size = 0, top = 0;
public:
    Reorder_Buffer rob[len];

    void reset() {
        for (int i = 0; i < len; i++)rob[i].Ready = false;
        size = 0, top = 0;
    }

    Reorder_Buffer head() {
        return rob[top];
    }

    void pop() {
        top = (top + 1) % len;
        size--;
    }

    bool available() {
        return size != len;
    }

    int getEmpty() {
        return (size + top) % len;
    }

    void insert(unsigned int order, int pc_now) {
        size++;
        int rear = (size - 1 + top) % len;
        rob[rear].Entry = rear;//指令编号
        rob[rear].Type = getType(order);
        rob[rear].PC_now = pc_now;
        rob[rear].Instruct = order;
        if (rob[rear].Type != S && rob[rear].Type != B) {
            rob[rear].Des = getRd(order);
            RegisterStat[rob[rear].Des].Reorder = rob[rear].Entry;
            RegisterStat[rob[rear].Des].Busy = true;
            rob[rear].Ready = false;
        }
    }

    void broadcast() {
        rob[CDB.entry].Ready = true;
        rob[CDB.entry].Value = CDB.result;
        rob[CDB.entry].PC_des = CDB.pc;
        unsigned order = getOpcode(rob[CDB.entry].Instruct);
        if (order == JALR || order == JAL)ISQ.reStart(rob[CDB.entry].PC_des);
    }

    void traverse() {
        for (int i = 0; i < size; i++) {
            int idx = (top + i) % len;
            std::cout << "Ready: " << rob[idx].Ready << " Order: " << rob[idx].Instruct << " PC: ";
            printf("%04x", rob[idx].PC_now);
            std::cout << " Result: " << rob[idx].Value << " Des: " << rob[idx].Des;
            std::cout << std::endl;
            getCommand(rob[idx].Instruct);
        }
    }

} ROB;

struct LS_Buffers {//load store buffer
private:
    static const int len = 8;
    struct LS_Buffer {// calculate address
        int State;
        unsigned int Op;
        unsigned int Vj, Vk;
        int Qj, Qk;
        unsigned int A;
        unsigned int Dest;
        unsigned int Address, Result;
        unsigned int clock;
    } LSB[len];

    struct LoadBus {
        int time = 0, i;
    } loadBus;

    struct StoreBus {
        int time = 0, i;
    } storeBus;
public:
    void reset() {
        for (int i = 0; i < len; i++) LSB[i].State = empty;
        loadBus.time = 0, storeBus.time = 0;
    }

    void insert(int entry, unsigned int order) {
        int i = 0;
        for (; i < len; i++)if (LSB[i].State == empty)break;
        LSB[i].Dest = entry, LSB[i].State = waiting, LSB[i].A = getImm(order), LSB[i].Op = order;
        LSB[i].clock = Clock;
        unsigned int opcode = getOpcode(order);
        int rs1 = getRs1(order);
        LSB[i].Qj = -1, LSB[i].Qk = -1;
        if (RegisterStat[rs1].Busy) {
            int h = RegisterStat[rs1].Reorder;
            if (ROB.rob[h].Ready)LSB[i].Vj = ROB.rob[h].Value;
            else LSB[i].Qj = h;
        } else LSB[i].Vj = reg[rs1];
        if (opcode == STORE) {
            int rs2 = getRs2(order);
            if (RegisterStat[rs2].Busy) {
                int h = RegisterStat[rs2].Reorder;
                if (ROB.rob[h].Ready)LSB[i].Vk = ROB.rob[h].Value;
                else LSB[i].Qk = h;
            } else LSB[i].Vk = reg[rs2];
        }
    }

    void commit(int entry) {
        for (int i = 0; i < len; i++) {
            if (LSB[i].Dest == entry) {
                LSB[i].State = storing;
                storeBus.time = 3;
                storeBus.i = i;
                break;
            }
        }
    }

    bool runStore() {
        if (!storeBus.time)return true;
        else {
            storeBus.time--;
            if (!storeBus.time) {
                int i = storeBus.i;
                Store(LSB[i].Op, LSB[i].Address, LSB[i].Result);
                LSB[i].State = empty;
                return true;
            }
        }
        return false;
    }

    void execute() {
        if (loadBus.time) {
            loadBus.time--;
            if (!loadBus.time)LSB[loadBus.i].State = finished;
        }
        for (int i = 0; i < len; i++)
            if (LSB[i].State == addressed) {
                int t = LSB[i].clock;
                bool flag = true;
                for (int j = 0; j < len; j++) {
                    if (LSB[j].State == waiting && getOpcode(LSB[j].Op) == STORE && LSB[j].clock < t) {
                        flag = false;
                        break;
                    }
                    if (LSB[j].State == waitingStore || LSB[j].State == storing || LSB[j].State == waitingBroadcast) {
                        if (LSB[i].Address == LSB[j].Address)
                            if (LSB[j].clock < t)
                                t = LSB[j].clock, LSB[i].Result = LSB[j].Result;
                    }
                }
                if (!flag)continue;
                if (t != LSB[i].clock) {
                    LSB[i].State = finished;
                    //          puts("Load");
                    //          std::cout << "Address:" << LSB[i].Address << " Result: " << LSB[i].Result << std::endl;
                    continue;
                } else {
                    if (loadBus.time)continue;
                    Load(LSB[i].Op, LSB[i].Address, LSB[i].Result);
                    //        puts("Load");
                    //        std::cout << "Address:" << LSB[i].Address << " Result: " << LSB[i].Result << std::endl;
                    LSB[i].State = loading;
                    loadBus.time = 3, loadBus.i = i;
                }
            }
        int i = 0;
        for (; i < len; i++)if (LSB[i].State == waiting && LSB[i].Qj == -1 && LSB[i].Qk == -1)break;
        if (i == len)return;
        unsigned int opcode = getOpcode(LSB[i].Op);
        if (opcode == LOAD) {
            getLoadAddress(LSB[i].Vj, LSB[i].A, LSB[i].Address);
            LSB[i].State = addressed;
        } else if (opcode == STORE) {
            FormatS(LSB[i].Op, LSB[i].A, LSB[i].Vj, LSB[i].Vk, LSB[i].Address, LSB[i].Result);
            LSB[i].State = waitingBroadcast;
        }
    }

    bool available() {
        for (int i = 0; i < len; i++)if (LSB[i].State == empty)return true;
        return false;
    }

    bool write() {
        for (int i = 0; i < len; i++) {
            if (LSB[i].State == finished) {
                CDB = (CommonDataBus) {LSB[i].Dest, LSB[i].Result};
                LSB[i].State = empty;
                return true;
            } else if (LSB[i].State == waitingBroadcast) {
                CDB = (CommonDataBus) {LSB[i].Dest, LSB[i].Result};
                LSB[i].State = waitingStore;
                return true;
            }
        }
        return false;
    }

    void broadcast() {
        for (int i = 0; i < len; i++) {
            if (LSB[i].State == waiting) {
                if (LSB[i].Qj == CDB.entry)LSB[i].Qj = -1, LSB[i].Vj = CDB.result;
                else if (LSB[i].Qk == CDB.entry)LSB[i].Qk = -1, LSB[i].Vk = CDB.result;
            }
        }
    }

} LSBuffer;

struct Reservation_Stations {//RS
private:
    static const int len = 8;
    struct Reservation_Station {
        int State;
        unsigned int Op;
        unsigned int Vj, Vk;//Vj rs1 Vk rs2
        int Qj, Qk;
        unsigned int A;
        unsigned int PC;
        unsigned int Dest;
        unsigned int Result;
    } rs[len];
public:
    bool available() {
        for (int i = 0; i < len; i++)if (rs[i].State == empty)return true;
        return false;
    }

    void reset() {
        for (int i = 0; i < len; i++)rs[i].State = empty;
    }

    void insert(int entry, unsigned int order) {
        int i = 0;
        for (; i < len; i++) if (rs[i].State == empty)break;
        rs[i].Dest = entry, rs[i].State = waiting, rs[i].Op = order, rs[i].PC = PC;
        rs[i].Qk = -1, rs[i].Qj = -1;
        int type = getType(order);
        if (type != R)rs[i].A = getImm(order);
        if (type == R || type == I || type == B) {
            int rs1 = getRs1(order);
            //      if (rs1 < 0 || rs1 > 31)puts("!");
            if (RegisterStat[rs1].Busy) {
                int h = RegisterStat[rs1].Reorder;
                if (ROB.rob[h].Ready)rs[i].Vj = ROB.rob[h].Value;
                else rs[i].Qj = h;
            } else rs[i].Vj = reg[rs1];
        }
        if (type == R || type == B) {
            int rs2 = getRs2(order);
            //       if (rs2 < 0 || rs2 > 31)puts("!");
            if (RegisterStat[rs2].Busy) {
                int h = RegisterStat[rs2].Reorder;
                if (ROB.rob[h].Ready)rs[i].Vk = ROB.rob[h].Value;
                else rs[i].Qk = h;
            } else rs[i].Vk = reg[rs2];
        }
    }

    void execute() {
        int i = 0;
        for (; i < len; i++)if (rs[i].State == waiting && rs[i].Qj == -1 && rs[i].Qk == -1)break;
        if (i == len)return;//not fund
        int type = getType(rs[i].Op);
        rs[i].State = finished;
        if (type == R)FormatR(rs[i].Op, rs[i].Vj, rs[i].Vk, rs[i].Result);
        else if (type == I)FormatI(rs[i].Op, rs[i].Vj, rs[i].A, rs[i].Result, rs[i].PC);
        else if (type == B)FormatB(rs[i].Op, rs[i].Vj, rs[i].Vk, rs[i].A, rs[i].Result, rs[i].PC);
        else if (type == U)FormatU(rs[i].Op, rs[i].A, rs[i].Result, rs[i].PC);
        else if (type == J)FormatJ(rs[i].Op, rs[i].A, rs[i].Result, rs[i].PC);
    }

    bool write() {
        for (int i = 0; i < len; i++) {
            //!B指令没有rd吧
            if (rs[i].State == finished) {
                if (getType(rs[i].Op) != B && getRd(rs[i].Op) == 0)rs[i].Result = 0;
                CDB = (CommonDataBus) {rs[i].Dest, rs[i].Result, rs[i].PC};
                rs[i].State = empty;
                return true;
            }
        }
        return false;
    }

    void broadcast() {
        for (int i = 0; i < len; i++) {
            if (rs[i].State == waiting) {
                if (rs[i].Qj == CDB.entry)rs[i].Qj = -1, rs[i].Vj = CDB.result;
                else if (rs[i].Qk == CDB.entry)rs[i].Qk = -1, rs[i].Vk = CDB.result;
            }
        }
    }

} RS;

void reset() {
    for (int i = 0; i <= 31; i++)RegisterStat[i].Busy = false;
    ROB.reset();
    RS.reset();
    LSBuffer.reset();
}

void Issue() {
    if (!ISQ.available()) {
        ISQ.GetInstructMem();
        return;
    }
    std::pair<unsigned, unsigned> t = ISQ.getCommand();
    unsigned int order = t.first, pc_now = t.second;
    ISQ.GetInstructMem();
    if (order == 0)return;
    if (getOpcode(order) == LOAD || getOpcode(order) == STORE) {
        if (!(LSBuffer.available() && ROB.available()))return;
        ISQ.pop();
        LSBuffer.insert(ROB.getEmpty(), order);
        ROB.insert(order, pc_now);
    } else {
        if (!(RS.available() && ROB.available()))return;
        ISQ.pop();
        RS.insert(ROB.getEmpty(), order);
        ROB.insert(order, pc_now);
    }
}

void Execute() {
    LSBuffer.execute();
    RS.execute();
}

void WriteResult() {
    if (!RS.write()) {
        if (!LSBuffer.write())return;
    }
    RS.broadcast();
    LSBuffer.broadcast();
    ROB.broadcast();
}

void Commit() {
    Reorder_Buffer rob = ROB.head();
    if (!LSBuffer.runStore())return;
    if (!rob.Ready)return;
    if (rob.Instruct == 0x0ff00513u) {
        std::cout << (reg[10] & 255u) << std::endl;
        exit(0);
    }
    if (rob.Type != S && rob.Type != B && rob.Des) {
        reg[rob.Des] = rob.Value;
        if (RegisterStat[rob.Des].Reorder == rob.Entry)RegisterStat[rob.Des].Busy = false;
    }
    if (rob.Type == S)LSBuffer.commit(rob.Entry);//可能还没有storing完就branch了？
    if (rob.Type == B ) {
        if (Predicter.jump(rob.PC_now) ^ rob.Value) {
            if (rob.Value)ISQ.reset(rob.PC_des);
            else ISQ.reset(rob.PC_now+4);
            reset();
        } else ROB.pop();
        Predicter.changeState(rob.Value, rob.PC_now);
    } else ROB.pop();
//    getCommand(rob.Instruct);
//    showReg();
}


#endif //TESTCASES_FOR_RISCV_TOMASULO_HPP
