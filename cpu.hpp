#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <bitset>
#include <iomanip>
#include "memory.hpp"

#define STACK_START 0x01FD
#define STACK_LEN 0x100
#define PC_START 0xC000
#define PROGRAM_OFFSET 0x0

// #define CARRY_FLAG(P) (P & 1)
// #define ZERO_FLAG(P) ((P & (1<<1))>>1)
// #define INTERR_FLAG(P) ((P & (1<<2))>>2)
// #define DECIM_FLAG(P) ((P & (1<<3))>>3)
// #define BREAK_FLAG(P) ((P & (1<<4))>>4)
// #define V_FLAG(P) ((P & (1<<6))>>6)
// #define NEG_FLAG(P) ((P & (1<<7))>>7)

#define CARRY 0
#define ZERO 1
#define INTERR 2
#define DECIM 3
#define BREAK 4
#define OVR 6
#define NEG 7

#define GET_FLAG(P,n) ((P & (1<<n))>>n)
// #define SET_FLAG(P,n) (GET_FLAG(P,n)==1) ? 0 : (1<<n)
// #define CLR_FLAG(P,n) (GET_FLAG(P,n)==1) ? (1<<n) : 0
#define SET_FLAG(flag,n) (flag==1) ? 0 : (1<<n)
#define CLR_FLAG(flag,n) (flag==1) ? (1<<n) : 0
#define CALC_CARRY(flag,val) (((val&0x100)>>8) ? (1 - flag) : (flag))
#define CALC_ZERO(flag,val) ((val==0) ? ((1 - flag)<<1) : (flag<<1))
#define CALC_OVR(flag,a,operand,res) (((((~a^operand)) & (a ^ res) & 0x80)== 0x80) ? ((1 - flag)<<6) : (flag<<6))

#define CALC_NEG(flag,val) ((val<0) ? ((1-flag)<<7) : (flag<<7))

#define CPU_ADDR_SPACE 65536

enum operand_k{
    immed,
    acc,
    addr
};

struct operand_t{
    uint16_t value;
    operand_k kind;
    uint16_t trace;
    bool page_boundary;
};

enum alu_op_codes{
    ORA_base = 0x00,
    AND_base = 0x20,
    EOR_base = 0x40,
    ADC_base = 0x60,
    STA_base = 0x80,
    LDA_base = 0xA0,
    CMP_base = 0xC0,
    SBC_base = 0xE0
};

enum rmw_op_codes{
    ASL_base = 0x00,
    ROL_base = 0x20,
    LSR_base = 0x40,
    ROR_base = 0x60
};

enum addr_mode{
    imm,        //2 Bytes
    zp,         //2 Bytes     
    zp_x,       //2 Bytes
    zp_y,       //2 Bytes
    rel,        //2 Bytes
    abs_,        //3 Bytes
    abs_x,      //3 Bytes
    abs_y,      //3 Bytes
    indir,      //3 Bytes
    indir_x,    //2 Bytes
    indir_y,    //2 Bytes
    accum,      //1 Bytes   
    impl        //1 Bytes   
};

class Cpu{
private:
    uint16_t PC; //program counter
    uint16_t S; //stack pointer
    uint8_t A; //accumulatort
    uint8_t X; //X index register
    uint8_t Y; //Y index register
    uint8_t P; //flags register (N V - B D I Z C)
    int cpu_cycle;
    int curr_cycle;
    void execute();
    void add_cycles(addr_mode mode, bool pg_crossed);
    uint8_t fetch_instr();
    uint16_t fetch_op(int bytes);
    operand_t decode_op(addr_mode _addr_mode, bool trace = false);
    uint8_t read_op(operand_t op);
public:
    Memory _mem;
    Cpu();
    //~Cpu();
    int test_file(char* filename);
    void cycle();

private:
    //CPU instructions
    void write(uint16_t i, uint8_t val);
    uint8_t read(uint16_t i);
    void ADC(addr_mode mode); // Add with Carry
    void AND(addr_mode mode); // Logical AND
    void ASL(addr_mode mode); // Arithmetic Shift Left
    void BCC(addr_mode mode); // Branch if Carry Clear
    void BCS(addr_mode mode); // Branch if Carry Set
    void BEQ(addr_mode mode); // Branch if Eaual
    void BIT(addr_mode mode); // Bit Test
    void BMI(addr_mode mode); // Branch if Minus
    void BNE(addr_mode mode); // Branch if Not Equal
    void BPL(addr_mode mode); // Branch if Positive
    void BRK(addr_mode mode); // Force Interrupt
    void BVC(addr_mode mode); // Branch if Overflow Clear
    void BVS(addr_mode mode); // Branch if Overflow Set
    void CLC(addr_mode mode); // Clear Carry Flag
    void CLD(addr_mode mode); // Clear Decimal Mode
    void CLI(addr_mode mode); // Clear Interrupt Disable
    void CLV(addr_mode mode); // Clear Overflow Flag
    void CMP(addr_mode mode); // Compare
    void CPX(addr_mode mode); // Compare X Register
    void CPY(addr_mode mode); // Compare Y Register
    void DEC(addr_mode mode); // Decrement Memory
    void DEX(addr_mode mode); // Decrement X Register
    void DEY(addr_mode mode); // Decrement Y Register
    void EOR(addr_mode mode); // Exclusive OR
    void INC(addr_mode mode); // Increment Memory
    void INX(addr_mode mode); // Increment X Register
    void INY(addr_mode mode); // Increment Y Register
    void JMP(addr_mode mode); // Jump
    void JSR(addr_mode mode); // Jump to Subroutine
    void LDA(addr_mode mode); // Load Accumulator
    void LDX(addr_mode mode); // Load X Register
    void LDY(addr_mode mode); // Load Y Register
    void LSR(addr_mode mode); // Logical Shift Right
    void NOP(addr_mode mode); // No Operation
    void ORA(addr_mode mode); // Logical Inclusive OR
    void PHA(addr_mode mode); // Push Accumulator
    void PHP(addr_mode mode); // Push Processor Status
    void PLA(addr_mode mode); // Pull Accumulator
    void PLP(addr_mode mode); // Pull Processor Status
    void ROL(addr_mode mode); // Rotate Left
    void ROR(addr_mode mode); // Rotate Right
    void RTI(addr_mode mode); // Return from Interrupt
    void RTS(addr_mode mode); // Return from Subroutine
    void SBC(addr_mode mode); // Subtract with Carry
    void SEC(addr_mode mode); // Set Carry Flag
    void SED(addr_mode mode); // Set Decimal Flag
    void SEI(addr_mode mode); // Set Interrupt Disable
    void STA(addr_mode mode); // Store Accumulator
    void STX(addr_mode mode); // Store X Register
    void STY(addr_mode mode); // Store Y Register
    void TAX(addr_mode mode); // Transfer Accumulator to X
    void TAY(addr_mode mode); // Transfer Accumulator to Y
    void TSX(addr_mode mode); // Transfer Stack Pointer to X
    void TXA(addr_mode mode); // Transfer X to Accumulator
    void TXS(addr_mode mode); // Transfer X to Stack Pointer
    void TYA(addr_mode mode); // Transfer Y to Accumulator

    uint16_t zp_indexed(uint16_t arg, uint8_t reg);
    uint16_t abs_indexed(uint16_t arg, uint8_t reg);
    uint16_t indirect(uint16_t arg);
    uint16_t dx(uint16_t arg);
    uint16_t dy(uint16_t arg);
    void trace(std::string op, addr_mode mode);

};

#endif