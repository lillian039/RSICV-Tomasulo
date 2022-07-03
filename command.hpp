#ifndef TESTCASES_FOR_RISCV_COMMAND_HPP
#define TESTCASES_FOR_RISCV_COMMAND_HPP

//Integer Register-Register Operations
void FormatR(unsigned int order, unsigned int rs1, unsigned int rs2, unsigned int &result) {
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int funct7 = get_num(31, 25, order);
    unsigned int id = funct3 + (funct7 >> 2);
    //all operations read the rs1 and rs2 registers as source operands
    //and write the result into register rd
    if (id == ADD) result = rs1 + rs2;
    else if (id == SUB)result = rs1 - rs2;
    else if (id == SLL)result = (rs1 << rs2);
    else if (id == SLT)result = (int(rs1) < int(rs2));
    else if (id == SLTU)result = (rs1 < rs2);
    else if (id == XOR)result = (rs1 ^ rs2);
    else if (id == SRL)result = (rs1 >> rs2);
    else if (id == SRA) {//shift right arithmetic :need to be signed
        unsigned int rd = (rs1 >> rs2);
        if (rs1 >> 31)for (int i = 31; i > 31 - rs2; i--)rd += (1 << i);
        result = rd;
    } else if (id == OR)result = (rs1 | rs2);
    else if (id == AND) result = (rs1 & rs2);
}

//register-immediate operations using I-type format
//loads are encoded in the I-type format
void FormatI(unsigned int order, unsigned int rs1, unsigned int imm, unsigned int &result, unsigned int &pc) {
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int funct7 = get_num(31, 25, order);
    unsigned int id = funct3 + (funct7 >> 2);
    unsigned int shamp = get_num(24, 20, order);
    unsigned int opcode = getOpcode(order);
    if (opcode == JALR) {//jump and link register
        result = pc + 4;
        pc = rs1 + imm;
    } else if (opcode == 0b0010011) {
        if (funct3 == ADDI) result = rs1 + imm;
        else if (funct3 == SLTI) result = (int(rs1) < int(imm));
        else if (funct3 == SLTIU) result = rs1 < imm;
        else if (funct3 == XORI)result = (rs1 ^ imm);
        else if (funct3 == ORI) result = (rs1 | imm);
        else if (funct3 == ANDI) result = (rs1 & imm);
        else if (id == SLLI) result = (rs1 << shamp);
        else if (id == SRLI) result = (rs1 >> shamp);
        else if (id == SRAI) {//shift right arithmetic Imm: need to be signed
            result = (rs1 >> shamp);
            if (rs1 >> 31)for (int i = 31; i > 31 - shamp; i--)result += (1 << i);
        }
    }
}

//Branch instructions
void FormatB(unsigned int order, unsigned int rs1, unsigned int rs2, unsigned int imm, unsigned int &result,
             unsigned int &pc) {
    unsigned int funct3 = get_num(14, 12, order);
    result = 0;
    //branch instructions compare two registers
    if (funct3 == BEQ) {//take the branch if register rs1 and rs2 are equal
        if (rs1 == rs2) result = 1, pc += imm;
    } else if (funct3 == BNE) {//take the branch if register rs1 and rs2 are unequal
        if (rs1 != rs2)result = 1, pc += imm;
    } else if (funct3 == BLT) {//branch less than
        if (int(rs1) < int(rs2))result = 1, pc += imm;
    } else if (funct3 == BGE) {//branch greater than or equal
        if (int(rs1) >= int(rs2))result = 1, pc += imm;
    } else if (funct3 == BLTU) {//branch less than unsigned
        if (rs1 < rs2)result = 1, pc += imm;
    } else if (funct3 == BGEU) {//branch greater than or equal unsigned
        if (rs1 >= rs2)result = 1, pc += imm;
    }
}

//upper immediate
void FormatU(unsigned int order, unsigned int imm, unsigned int &result, unsigned int pc) {
    unsigned int opcode = getOpcode(order);
    if (opcode == LUI) {//Load From Immediate
        result = imm;
    } else if (opcode == AUIPC) {//Add Upper Immediate to PC
        result = imm + pc;
    }
}

//jump
void FormatJ(unsigned int order, unsigned int imm, unsigned int &result, unsigned int &pc) {
    unsigned int opcode = getOpcode(order);
    if (opcode == JAL) {// jump and link
        result = pc + 4;
        pc = pc + imm;
    }
}

//load
void getLoadAddress(unsigned int rs1, unsigned int imm, unsigned int &address) {
    address = rs1 + imm;
}

//store alu
void FormatS(unsigned int order, unsigned int imm, unsigned int rs1, unsigned int rs2, unsigned int &address,
             unsigned int &result) {
    unsigned int funct3 = get_num(14, 12, order);
    //the effective address is obtained by
    // adding register rs1 to the sign-extended 12-bits offset
    address = rs1 + imm;
    //store 8-bit from low bits of register rs2 to memory
    if (funct3 == SB) result = (rs2 & 255);
    //store 16-bit from low bits of register rs2 to memory
    else if (funct3 == SH)result = (rs2 & ((1 << 17) - 1));
    //store 32-bit from low bits of register rs2 to memory
    else if (funct3 == SW)result = rs2;
}

void Load(unsigned int order, unsigned int address, unsigned int &result) {
    unsigned int funct3 = get_num(14, 12, order);
    if (address >= 1e6)return;
    if (funct3 == LB) {
        unsigned int bit_8 = mem[address];
        if (bit_8 >> 7)for (int i = 31; i >= 8; i--)bit_8 += (1 << i);
        result = bit_8;
    }
        //load 16 byte value form memory
        //than signed-extend to 32-bits before stording in rd;
    else if (funct3 == LH) {
        unsigned int bit_16 = mem[address] + (mem[address + 1] << 8);
        if (bit_16 >> 15)for (int i = 31; i >= 16; i--)bit_16 += (1 << i);
        result = bit_16;
    } else if (funct3 == LW) {
        unsigned int bit_32 =
                mem[address] | (mem[address + 1] << 8) | (mem[address + 2] << 16) | (mem[address + 3] << 24);
        result = bit_32;
    } else if (funct3 == LBU) {//load byte unsigned
        //load 8 byte value form memory
        //than zero-extend to 32-bits before stording in rd;
        unsigned int bit_8 = (mem[address] & ((1 << 9) - 1));
        result = bit_8;
    } else if (funct3 == LHU) {
        //load 16 byte value form memory
        //than zero-extend to 32-bits before stording in rd;
        unsigned int bit_16 = mem[address] + (mem[address + 1] << 8);
        result = bit_16;
    }
}

void Store(unsigned int order, unsigned int address, unsigned int result) {
    unsigned int funct3 = get_num(14, 12, order);
    //the effective address is obtained by
    // adding register rs1 to the sign-extended 12-bits offset
    //store 8-bit from low bits of register rs2 to memory
    if (funct3 == SB) mem[address] = result;
    //store 16-bit from low bits of register rs2 to memory
    else if (funct3 == SH) {
        mem[address] = (result >> 8);
        mem[address + 1] = (result & 255u);
    }
        //store 32-bit from low bits of register rs2 to memory
    else if (funct3 == SW) {
        mem[address + 3] = (result >> 24);
        mem[address + 2] = ((result >> 16) & 255u);
        mem[address + 1] = ((result >> 8) & 255u);
        mem[address] = (result & 255u);
    }
}

#endif //TESTCASES_FOR_RISCV_COMMAND_HPP
