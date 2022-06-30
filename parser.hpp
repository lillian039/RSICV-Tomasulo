#ifndef TESTCASES_FOR_RISCV_PARSER_HPP
#define TESTCASES_FOR_RISCV_PARSER_HPP

unsigned int get_num(const int &l, const int &r, const unsigned int &order) {//get l - r num 0 base
    return ((order & ((1ll << (l + 1)) - 1)) >> r);
}

enum Type {
    R = 1, I = 2, S = 3, B = 4, U = 5, J = 6
};

enum operaType {
    LUI = 0b0110111,//Load From Immediate
    AUIPC = 0b0010111,//Add Upper Immediate to PC
    JAL = 0b1101111,//Jump & Link
    JALR = 0b1100111,//Jump & Link Register
    BEQ = 0b000,//Branch Equal
    BNE = 0b001,//Branch Not Equal
    BLT = 0b100,//Branch Less Than
    BGE = 0b101,//Branch Greater than or Equal
    BLTU = 0b110,//Branch Less Than Unsigned
    BGEU = 0b111,//Branch >= Unsigned
    LB = 0b000,// Load Byte
    LH = 0b001,
    LW = 0b010,
    LBU = 0b100,
    LHU = 0b101,
    SB = 0b000,
    SH = 0b001,
    SW = 0b010,
    ADDI = 0b000,
    SLTI = 0b010,
    SLTIU = 0b011,
    XORI = 0b100,
    ORI = 0b110,
    ANDI = 0b111,
    SLLI = 0b001,
    SRLI = 0b101 + (0b0 << 3),
    SRAI = 0b101 + (0b1 << 3),
    ADD = 0b000 + (0b0 << 3),
    SUB = 0b000 + (0b1 << 3),
    SLL = 0b001,
    SLT = 0b010,
    SLTU = 0b011,
    XOR = 0b100,
    SRL = 0b101 + (0b0 << 3),
    SRA = 0b101 + (0b1 << 3),
    OR = 0b110,
    AND = 0b111,
    LOAD = 0b0000011,
    STORE = 0b0100011
};

int getType(const unsigned int &order) {
    unsigned int opcode = (order & 127);
    if (opcode == LUI || opcode == AUIPC)return U;
    else if (opcode == JAL)return J;
    else if (opcode == 0b1100011)return B;
    else if (opcode == 0b0010011 || opcode == 0b0000011 || opcode == JALR)return I;
    else if (opcode == 0b0100011)return S;
    else if (opcode == 0b0110011)return R;
    return -1;
}

unsigned int getRd(unsigned int order) {
    return get_num(11, 7, order);
}

unsigned int getRs1(unsigned int order) {
    return get_num(19, 15, order);
}

unsigned int getRs2(const unsigned int &order) {
    return get_num(24, 20, order);
}

unsigned int getFunct3(const unsigned int &order) {
    return get_num(14, 12, order);
}

unsigned int getFunct7(const unsigned int &order) {
    return get_num(31, 25, order);
}

unsigned int getShamp(const unsigned int &order) {
    return get_num(24, 20, order);
}

unsigned int getImm(const unsigned int &order) {
    unsigned int imm;
    int type = getType(order);
    if (type == I) {
        imm = (order >> 20);//[31,20]
        if (order >> 31)for (int i = 31; i >= 12; i--)imm += (1 << i);
    } else if (type == S) {
        imm = get_num(11, 7, order) + (get_num(30, 25, order) << 5);//[31,20]
        if (order >> 31)for (int i = 31; i >= 11; i--)imm += (1 << i);
    } else if (type == B) {
        imm = (get_num(11, 8, order) << 1) + (get_num(30, 25, order) << 5) + (get_num(7, 7, order) << 11);
        if (order >> 31)for (int i = 31; i >= 12; i--)imm += (1 << i);
    } else if (type == U) {
        imm = (get_num(31, 12, order) << 12);
    } else if (type == J) {
        imm = (get_num(20, 20, order) << 11) + (get_num(30, 21, order) << 1) + (get_num(19, 12, order) << 12);
        if (order >> 31)for (int i = 31; i >= 20; i--)imm += (1 << i);
    }
    return imm;
}

unsigned int getOpcode(const unsigned int &order) {
    return (order & 127);
}

void showReg() {
    for (int i = 0; i < 32; i++) {
        std::cout << reg[i] << " ";
    }
    std::cout << std::endl;
}

void getCommand(const unsigned int &order) {
    if (order == 0x0ff00513u) {
        puts("exit");
    }
    int opcode = (order & 127);
    unsigned int funct3 = getFunct3(order);
    unsigned int funct7 = getFunct7(order);
    unsigned int id = funct3 + (funct7 >> 2);
    if (opcode == LUI)puts("Lui");
    else if (opcode == AUIPC)puts("Auipc");
    else if (opcode == JAL)puts("Jal");
    else if (opcode == 0b1100011) {
        if (funct3 == BEQ)puts("Beq");
        else if (funct3 == BNE)std::cout<<"Bne! ";
        else if (funct3 == BLT)std::cout<<"Blt! ";
        else if (funct3 == BGE)std::cout<<"Bge! ";
        else if (funct3 == BLTU)std::cout<<"Bltu! ";
        else if (funct3 == BGEU)std::cout<<"Bgeu! ";
    } else if (opcode == 0b0010011) {
        if (funct3 == ADDI) puts("Addi");
        else if (funct3 == SLTI) puts("Slti");
        else if (funct3 == SLTIU) puts("Sltiu");
        else if (funct3 == XORI) puts("Xori");
        else if (funct3 == ORI) puts("ori");
        else if (funct3 == ANDI) puts("Andi");
        else if (id == SLLI)puts("Slli");
        else if (id == SRLI) puts("Srli");
        else if (id == SRAI) puts("Srai");
    } else if (opcode == 0b0000011) {
        if (funct3 == LB) puts("Lb");
        else if (funct3 == LH) puts("Lh");
        else if (funct3 == LW) puts("Lw");
        else if (funct3 == LBU)puts("Lbu");
        else if (funct3 == LHU) puts("Lhu");
    } else if (opcode == JALR)puts("Jalr");
    else if (opcode == 0b0100011) {
        if (funct3 == SB) puts("Sb");
        else if (funct3 == SH) puts("Sh");
        else if (funct3 == SW) puts("Sw");
    } else if (opcode == 0b0110011) {
        if (id == ADD) puts("Add");
        else if (id == SUB) puts("Sub");
        else if (id == SLL) puts("Sll");
        else if (id == SLT) puts("Slt");
        else if (id == SLTU) puts("Sltu");
        else if (id == XOR)puts("Xor");
        else if (id == SRL)puts("Srl");
        else if (id == SRA) puts("Sra");
        else if (id == OR)puts("Or");
        else if (id == AND) puts("And");
    }
}


#endif //TESTCASES_FOR_RISCV_PARSER_HPP
