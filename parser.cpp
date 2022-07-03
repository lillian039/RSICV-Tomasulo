#include <iostream>
#include <cstdio>
#include <map>

using namespace std;
unsigned int regis[32], mem[1000000], des = 0, PC = 0;

void to_binary(unsigned int x, int len) {
    for (int i = len - 1; i >= 0; i--)cout << ((x >> i) & 1);
    cout << endl;
}

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
};

unsigned int get_num(int l, int r, unsigned int order) {//get l - r num 0 base
    unsigned int ans = ((order & ((1ll << (l + 1)) - 1)) >> r);
    return ans;
}

unsigned int get_int(char first) {
    int power[8] = {4, 0, 12, 8, 20, 16, 28, 24};
    unsigned int cmd = 0;
    for (int i = 0; i < 8; i++) {
        if ('0' <= first && first <= '9')cmd += (first - '0') * (1 << power[i]);
        else cmd += (first - 'A' + 10) * (1 << power[i]);
        if (i <= 6)cin >> first;
    }
    // first = getchar();
    return cmd;
}

unsigned int me_to_int(string num) {
    unsigned int d = 0;
    int power[8] = {28, 24, 20, 16, 12, 8, 4, 0};
    for (int i = 0; i <= 7; i++) {
        if ('0' <= num[i] && num[i] <= '9')d += (num[i] - '0') * (1 << power[i]);
        else d += (num[i] - 'A' + 10) * (1 << power[i]);
    }
    return d;
}

//Integer Register-Register Operations
void FormatR(unsigned int order, int opcode) {
    unsigned int rd = get_num(11, 7, order);
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int rs1 = get_num(19, 15, order);
    unsigned int rs2 = get_num(24, 20, order);
    unsigned int funct7 = get_num(31, 25, order);
    unsigned int id = funct3 + (funct7 >> 2);
    //all operations read the rs1 and rs2 registers as source operands
    //and write the result into register rd
    if (opcode == 0b0110011) {
        if (id == ADD) {
            puts("Add");
            regis[rd] = regis[rs1] + regis[rs2];
        } else if (id == SUB) {
            puts("Sub");
            regis[rd] = regis[rs1] - regis[rs2];
        } else if (id == SLL) {//shift left
            puts("Sll");
            regis[rd] = (regis[rs1] << regis[rs2]);
        } else if (id == SLT) {//set less than
            puts("Slt");
            regis[rd] = int(regis[rs1]) < int(regis[rs2]);
        } else if (id == SLTU) {
            puts("Sltu");
            regis[rd] = regis[rs1] < regis[rs2];
        } else if (id == XOR) {
            puts("Xor");
            //       cout << "xor" << endl;
            regis[rd] = (regis[rs1] ^ regis[rs2]);
        } else if (id == SRL) {//shift right
            puts("Srl");
            regis[rd] = (regis[rs1] >> regis[rs2]);
        } else if (id == SRA) {//shift right arithmetic :need to be signed
            puts("Sra");
            regis[rd] = (regis[rs1] >> regis[rs2]);
            if (regis[rs1] >> 31)for (int i = 31; i > 31 - regis[rs2]; i--)regis[rd] += (1 << i);
        } else if (id == OR) {
            puts("Or");
            regis[rd] = (regis[rs1] | regis[rs2]);
        } else if (id == AND) {
            puts("And");
            regis[rd] = (regis[rs1] & regis[rs2]);
        }
    }
    PC += 4;
}

//register-immediate operations using I-type format
//loads are encoded in the I-type format
void FormatI(unsigned int order, int opcode) {
    unsigned int rd = get_num(11, 7, order);
    unsigned int rs1 = get_num(19, 15, order);
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int funct7 = get_num(31, 25, order);
    unsigned int id = funct3 + (funct7 >> 2);
    unsigned int shamp = get_num(24, 20, order);
    unsigned int imm = (order >> 20);//[31,20]
    if (order >> 31)for (int i = 31; i >= 12; i--)imm += (1 << i);
    if (opcode == JALR) {//jump and link register
        puts("Jalr");
        regis[rd] = PC + 4;
        PC = regis[rs1] + imm;
        //   cout<<"rs1: "<<rs1<<" regis[rs1]: "<<regis[rs1]<<" imm: "<<imm<<endl;
        //   printf("%04x",PC);
        //    cout<<endl;
    } else if (opcode == 0b0000011) {
        //the effective address is obtained by
        // adding register rs1 to the sign-extended 12-bits offset
        unsigned int address = regis[rs1] + imm;
        //load 8 byte value form memory
        //than signed-extend to 32-bits before stording in rd;
        if (funct3 == LB) {
            puts("Lb");
            unsigned int bit_8 = mem[address];
            if (bit_8 >> 7)for (int i = 31; i >= 8; i--)bit_8 += (1 << i);
            regis[rd] = bit_8;
        }
            //load 16 byte value form memory
            //than signed-extend to 32-bits before stording in rd;
        else if (funct3 == LH) {
            puts("Lh");
            unsigned int bit_16 = mem[address] + (mem[address + 1] << 8);
            if (bit_16 >> 15)for (int i = 31; i >= 16; i--)bit_16 += (1 << i);
            regis[rd] = bit_16;
        } else if (funct3 == LW) {
            puts("Lw");
            unsigned int bit_32 =
                    mem[address] | (mem[address + 1] << 8) | (mem[address + 2] << 16) | (mem[address + 3] << 24);
            regis[rd] = bit_32;
     //            cout<<"Address: "<<address<<" Result: "<<bit_32<<endl;
        } else if (funct3 == LBU) {//load byte unsigned
            //load 8 byte value form memory
            //than zero-extend to 32-bits before stording in rd;
            puts("Lbu");
            unsigned int bit_8 = (mem[address] & ((1 << 9) - 1));
            regis[rd] = bit_8;
        } else if (funct3 == LHU) {
            puts("Lhu");
            //load 16 byte value form memory
            //than zero-extend to 32-bits before stording in rd;
            unsigned int bit_16 = mem[address] + (mem[address + 1] << 8);
            regis[rd] = bit_16;
        }
        PC += 4;
    } else if (opcode == 0b0010011) {
        if (funct3 == ADDI) {//add immediate(word)
            puts("Addi");
            regis[rd] = regis[rs1] + imm;
          //          std::cout <<"rd: "<<rd<<" rs1: "<<rs1<<" regis[rs1]: " << regis[rs1]  << " imm: " << imm << std::endl;
        } else if (funct3 == SLTI) {//set less than immediate
            puts("Slti");
            regis[rd] = (int(regis[rs1]) < int(imm));
        } else if (funct3 == SLTIU) {//set < immediate unsigned
            puts("Sltiu");
            regis[rd] = regis[rs1] < imm;
        } else if (funct3 == XORI) {//XOR immediate
            puts("Xori");
            regis[rd] = (regis[rs1] ^ imm);
        } else if (funct3 == ORI) {// OR immediate
            puts("ori");
            regis[rd] = (regis[rs1] | imm);
        } else if (funct3 == ANDI) {// AND immediate
            puts("Andi");
            regis[rd] = (regis[rs1] & imm);
        } else if (id == SLLI) {//shift left immediate
            puts("Slli");
            regis[rd] = (regis[rs1] << shamp);
        } else if (id == SRLI) {//shift right immediate
            puts("Srli");
            regis[rd] = (regis[rs1] >> shamp);
        } else if (id == SRAI) {//shift right arithmetic Imm: need to be signed
            puts("Srai");
            regis[rd] = (regis[rs1] >> shamp);
            if (regis[rs1] >> 31)for (int i = 31; i > 31 - shamp; i--)regis[rd] += (1 << i);
        }
        PC += 4;
    }
}

//loads are stored int S type
void FormatS(unsigned int order, int opcode) {
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int rs1 = get_num(19, 15, order);
    unsigned int rs2 = get_num(24, 20, order);
    unsigned int imm = get_num(11, 7, order) + (get_num(30, 25, order) << 5);//[31,20]
    if (order >> 31)for (int i = 31; i >= 11; i--)imm += (1 << i);
    //the effective address is obtained by
    // adding register rs1 to the sign-extended 12-bits offset
    unsigned int address = regis[rs1] + imm;
    //store 8-bit from low bits of register rs2 to memory
    if (funct3 == SB) {
        puts("Sb");
        unsigned int bit_8 = (regis[rs2] & 255);
        mem[address] = bit_8;
    }
        //store 16-bit from low bits of register rs2 to memory
    else if (funct3 == SH) {
        puts("Sh");
        unsigned int bit_16 = (regis[rs2] & ((1 << 17) - 1));
        mem[address] = (bit_16 >> 8);
        mem[address + 1] = (bit_16 & 255u);
    }
        //store 32-bit from low bits of register rs2 to memory
    else if (funct3 == SW) {
        puts("Sw");
        unsigned int bit_32 = regis[rs2];
   //         cout<<"Address: "<<address<<" Result: "<<bit_32<<endl;
        mem[address + 3] = (bit_32 >> 24);
        mem[address + 2] = ((bit_32 >> 16) & 255u);
        mem[address + 1] = ((bit_32 >> 8) & 255u);
        mem[address] = (bit_32 & 255u);
    }
    PC += 4;
}

//Branch instructions
void FormatB(unsigned int order, int opcode) {
    unsigned int funct3 = get_num(14, 12, order);
    unsigned int rs1 = get_num(19, 15, order);
    unsigned int rs2 = get_num(24, 20, order);
    unsigned int imm = (get_num(11, 8, order) << 1) + (get_num(30, 25, order) << 5) + (get_num(7, 7, order) << 11);
    if (order >> 31)for (int i = 31; i >= 12; i--)imm += (1 << i);
    // cout<<imm<<endl;
    //branch instructions compare two registers
    if (funct3 == BEQ) {//take the branch if register rs1 and rs2 are equal
        puts("Beq");
    //    cout<<"rs1: "<<rs1<<" rs2: "<<rs2<<" regis[rs1]: "<<regis[rs1]<<" regis[rs2]: "<<regis[rs2]<<endl;
        if (regis[rs1] == regis[rs2]) {
            PC = PC + imm - 4;
          //  printf("%04x ",PC);
        }
    //    else   std::cout << "Beq! PC: " << PC + 4<<'\n';
    } else if (funct3 == BNE) {//take the branch if register rs1 and rs2 are unequal
        puts("Bne");
        if (regis[rs1] != regis[rs2]) { PC = PC + imm - 4; }
    //    else std::cout << "Bne! PC: " << PC + 4<<'\n';
    } else if (funct3 == BLT) {//branch less than
        puts("Blt");
        //      cout<<"rs1: "<<rs1<<" rs2: "<<rs2<<" regis[rs1]: "<<regis[rs1]<<" regis[rs2]: "<<regis[rs2]<<endl;
        if (int(regis[rs1]) < int(regis[rs2])){ PC = PC + imm - 4; }
     //   else std::cout << "Blt! PC: " << PC + 4<<'\n';
    } else if (funct3 == BGE) {//branch greater than or equal
        puts("Bge");
        if (int(regis[rs1]) >= int(regis[rs2])){ PC = PC + imm - 4; }
     //   else std::cout << "Bge! PC: " << PC + 4<<'\n';
    } else if (funct3 == BLTU) {//branch less than unsigned
        puts("Bltu");
        if (regis[rs1] < regis[rs2]){ PC = PC + imm - 4; }
     //   else std::cout << "Bltu! PC: " << PC + 4<<'\n';
    } else if (funct3 == BGEU) {//branch greater than or equal unsigned
        puts("Bgeu");
        if (regis[rs1] >= regis[rs2]){ PC = PC + imm - 4; }
    //    else std::cout << "Bgeu! PC: " << PC + 4<<'\n';
    }
    PC += 4;
}

//upper immediate
void FormatU(unsigned int order, int opcode) {
    unsigned int rd = get_num(11, 7, order);
    unsigned int imm = (get_num(31, 12, order) << 12);
    if (opcode == LUI) {//Load From Immediate
        puts("Lui");
        regis[rd] = imm;
    } else if (opcode == AUIPC) {//Add Upper Immediate to PC
        puts("Auipc");
        regis[rd] = imm + PC;
    }
    PC += 4;
}

//jump
void FormatJ(unsigned int order, int opcode) {
    unsigned int rd = get_num(11, 7, order);
    unsigned int imm = (get_num(20, 20, order) << 11) + (get_num(30, 21, order) << 1) + (get_num(19, 12, order) << 12);
    if (order >> 31)for (int i = 31; i >= 20; i--)imm += (1 << i);
    if (opcode == JAL) {// jump and link
        puts("Jal");
        regis[rd] = PC + 4;
        PC = PC + imm;
    }
}

void getReg() {
    for (int i = 0; i < 32; i++) {
        std::cout << regis[i] << " ";
    }
    std::cout << std::endl;
}

void parser(unsigned int order) {
    if (order == 0x0ff00513u) {
        cout << (regis[10] & 255u) << endl;
        getReg();
        exit(0);
    }
    int opcode = (order & 127);
    if (opcode == LUI || opcode == AUIPC)FormatU(order, opcode);
    else if (opcode == JAL)FormatJ(order, opcode);
    else if (opcode == 0b1100011)FormatB(order, opcode);
    else if (opcode == 0b0010011 || opcode == 0b0000011 || opcode == JALR)FormatI(order, opcode);
    else if (opcode == 0b0100011)FormatS(order, opcode);
    else if (opcode == 0b0110011)FormatR(order, opcode);
}

void set_memory(char first) {
    int power[8] = {4, 0, 4, 0, 4, 0, 4, 0};
    for (int i = 0; i < 8; i++) {
        if ('0' <= first && first <= '9')mem[des + i / 2] += (first - '0') * (1 << power[i]);
        else mem[des + i / 2] += (first - 'A' + 10) * (1 << power[i]);
        if (i <= 6)cin >> first;
    }
}

int main() {
    freopen("bulgarian.data", "r", stdin);
    freopen("ans.out", "w", stdout);
    string num;
    char first_char;
    unsigned int order;
    while (cin >> first_char) {
        if (first_char == '@') {
            cin >> num;
            des = me_to_int(num);
        } else {
            set_memory(first_char);
            des += 4;
        }
    }
    int i = 0;
    while (1) {
        i++;
             printf("%04x ",PC);
        // cout << (regis[10] & 255u) << endl;
        // cout<<"regis[1]: "<<regis[1]<<endl;
        order = (mem[PC] & ((1 << 9) - 1)) + ((mem[PC + 1] & ((1 << 17) - 1)) << 8) +
                ((mem[PC + 2] & ((1 << 25) - 1)) << 16) + (mem[PC + 3] << 24);
        //    cout<<order<<" ";

        parser(order);
        if (PC == 0x12ac) {
            getReg();
            //    ROB.traverse();
        }
        if (PC == 0x1284) {
            getReg();
           /* puts("!");
            std::cout<<rob.Instruct<<std::endl;
            showReg();*/
            //    ROB.traverse();
        }
        regis[0] = 0;
     //
    }
}
