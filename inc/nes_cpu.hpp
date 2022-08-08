#ifndef NES_CPU_HPP_
#define NES_CPU_HPP_

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <bitset>
#include <iomanip>
#include "nes_memory.hpp"
#include "nes_system.hpp"
#include "nes_component.hpp"
#include "helper.hpp"

#define STACK_START 0x01FD
#define STACK_LEN 0x100
#define PC_START 0xC000
#define PROGRAM_OFFSET 0x0
#define CPU_ADDR_SPACE 65536

/*
    3 possible operand types
*/
enum operand_k{
    immed,
    acc,
    addr
};

/*
    Contains necessary information about a retrieved operand
*/
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

/*
    NES addressing modes
*/
enum addr_mode{
    imm,        // 2 Bytes
    zp,         // 2 Bytes     
    zp_x,       // 2 Bytes
    zp_y,       // 2 Bytes
    rel,        // 2 Bytes
    abs_,       // 3 Bytes
    abs_x,      // 3 Bytes
    abs_y,      // 3 Bytes
    indir,      // 3 Bytes
    indir_x,    // 2 Bytes
    indir_y,    // 2 Bytes
    accum,      // 1 Bytes   
    impl        // 1 Bytes   
};

// struct dma_request{
//     bool pending;
//     uint8_t hi_byte;
// };

class nes_system;
class nes_memory;

class nes_cpu: public nes_component{
private:
    uint16_t PC;    // program counter
    uint16_t SP;     // stack pointer
    uint8_t A;      // accumulatort
    uint8_t X;      // X index register
    uint8_t Y;      // Y index register
    uint8_t P;      // flags register (N V - B D I Z C)
    bool nmi_pending;
    bool dma_pending;
    bool reset_pending;
    //dma_request dma_req;
    int cpu_cycle;
    int curr_cycle;

    nes_system *_nes_system;
      
    void execute();
    uint8_t fetch_instr();
    uint16_t fetch_op(int bytes);
    operand_t decode_op(addr_mode _addr_mode, bool trace = false);
    uint8_t read_op(operand_t op);
    void write(uint16_t i, uint8_t val);
    uint8_t read(uint16_t i);

public:
    nes_memory* _mem;  
    nes_cpu();
    ~nes_cpu();
    //void cycle();
    virtual void turn_on(nes_system *sys);
    virtual void reset();
    virtual void step_to(uint64_t master_cycle);
    void set_NMI();
    void NMI();
    //void set_dma_req(uint8_t val);
    void set_dma_req();
    void DMA();

private:
    //6502 CPU Instruction Set

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

    // addressing modes' formulas
    
    uint16_t zp_indexed(uint16_t arg, uint8_t reg);     //ZeroPage/ZeroPage,X/ZeroPage,Y modes
    uint16_t abs_indexed(uint16_t arg, uint8_t reg);    //Absolute/Absolute,X/Absolute,Y modes
    uint16_t indirect(uint16_t arg);                    //indirect mode  
    uint16_t dx(uint16_t arg);                          //indexed indirect mode
    uint16_t dy(uint16_t arg);                          //indirect index mode

    // debug
    void op_trace(std::string op, addr_mode mode);

};

#endif