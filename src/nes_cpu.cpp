#include "../inc/nes_cpu.hpp"

nes_cpu::nes_cpu(){
    A = 0;
    X = 0;
    Y = 0;
    P = 0x24;
    PC = PC_START;
    SP = STACK_START;
    cpu_cycle = 7;
    curr_cycle = 0;
    _mem = nullptr;
    _nes_system = nullptr;
    nmi_pending = false;
    dma_pending = false;
}

nes_cpu::~nes_cpu(){}

void nes_cpu::turn_on(nes_system *sys){
    _nes_system = sys;
    _mem = sys->mem();
    reset_pending = true;
}

void nes_cpu::reset(){
    PC = read(0xFFFC) | (read(0xFFFD)<<8);
    //P = (1<<5);
    P = 0x20;
    SP = 0x1FD;
    A = 0;
    X = 0;
    Y = 0;


}

void nes_cpu::step_to(uint64_t master_cycle){
    while(cpu_cycle <= master_cycle){
        execute();
    }
}
/*
    Memory read wrapper that adds a cycle.
*/
uint8_t nes_cpu::read(uint16_t i){
    curr_cycle += 1;
    return _mem->PEEK(i);
}

/*
    Memory write wrapper that adds a cycle.
*/
void nes_cpu::write(uint16_t i, uint8_t val){
    curr_cycle += 1;
    _mem->SET(i,val);
}

/*
    Fetches instruciton opcode.
*/
uint8_t nes_cpu::fetch_instr(){
    return read(PC++);
}

/*
    Fetches extra bytes in little endian form.
*/
uint16_t nes_cpu::fetch_op(int bytes){
    uint16_t result = 0;

    for(int i = 0; i<bytes; i++)
        result |= (read(PC++) << (8*i));
    
    return result;
}

/*
    Fetches extra bytes for current instruction.
    Determines address/value depending on addressing mode.
*/
operand_t nes_cpu::decode_op(addr_mode mode, bool trace/*=false*/){

    operand_t operand;
    operand.page_boundary = false;

    switch(mode){
        case addr_mode::accum:
        case addr_mode::impl:
            operand.kind = operand_k::acc;
            if(trace) std::cout<<"       ";
            break;
        
        case addr_mode::imm:
        case addr_mode::rel:
            operand.kind = operand_k::immed;
            operand.value = fetch_op(1);

            if(trace){ 
                PC -= 1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.value&0x00FF)<<"     ";
            }

            break;
        
        case addr_mode::zp:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(1);
            operand.value = zp_indexed(operand.trace,0);

            if(trace) {
                PC -= 1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace&0x00FF)<<"     ";
            }

            break;
        
        case addr_mode::zp_x:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(1);
            operand.value = zp_indexed(operand.trace,X);

            if(trace) {
                PC -= 1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace&0x00FF)<<"     ";
            }

            curr_cycle += 1;
            break;
        
        case addr_mode::zp_y:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(1);
            operand.value = zp_indexed(operand.trace,Y);

            if(trace) {
                PC -= 1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace&0x00FF)<<"     ";
            }

            curr_cycle += 1;
            break;
        
        case addr_mode::abs_:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(2);
            operand.value = abs_indexed(operand.trace,0);

            if(trace){ 
                PC-=2;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<" "<<
                    std::setw(2)<<(int)((operand.trace & 0xFF00)>>8)<<"  ";
            }

            break;
        
        case addr_mode::abs_x:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(2);
            operand.value = abs_indexed(operand.trace,X);

            if(trace){ 
                PC-=2;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<" "<<
                    std::setw(2)<<(int)((operand.trace & 0xFF00)>>8)<<"  ";
            }

            if((operand.value & 0xFF00) != (operand.trace & 0xFF00))
                operand.page_boundary = true;
            
            break;
        
        case addr_mode::abs_y:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(2);
            operand.value = abs_indexed(operand.trace,Y);

            if(trace){ 
                PC-=2;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<" "<<
                    std::setw(2)<<(int)((operand.trace & 0xFF00)>>8)<<"  ";
            }

            if((operand.value & 0xFF00) != (operand.trace & 0xFF00))
                operand.page_boundary = true;
            
            break;
        
        case addr_mode::indir:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(2);
            operand.value = indirect(operand.trace);

            if(trace){ 
                PC-=2;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<" "<<
                    std::setw(2)<<(int)((operand.trace & 0xFF00)>>8)<<"  ";
            }

            break;
        
        case addr_mode::indir_x:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(1);
            operand.value = dx(operand.trace);

            if(trace){ 
                PC-=1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<"     ";
            }

            curr_cycle += 1;
            break;
        
        case addr_mode::indir_y:
            operand.kind = operand_k::addr;
            operand.trace = fetch_op(1);
            operand.value = dy(operand.trace);

            if(trace){ 
                PC-=1;
                std::cout<<std::uppercase<<std::hex<<std::setw(2)<<(int)(operand.trace & 0x00FF)<<"     ";
            }

            if((operand.value & 0xFF00) != ((operand.value-Y) & 0xFF00))
                operand.page_boundary = true;
            
            break;
    }
    
    return operand;

}

/*
    Return correct value from decoded operand
*/
uint8_t nes_cpu::read_op(operand_t operand){

    switch(operand.kind){

        case operand_k::acc:
            return A;
        
        case operand_k::addr:
            return read(operand.value);
        
        case operand_k::immed:
            return operand.value;
        
        default:
            std::cout<<"ERROR in read_op()"<<std::endl;
            exit(1);
        
    }
    return 0;
}


/*
    Full instruction fetch/decode/execute cycle
*/
void nes_cpu::execute(){

    
    curr_cycle = 0;
    P ^= SET_FLAG(GET_FLAG(P,BFLAG),BFLAG);

    if(reset_pending){
        reset();
        cpu_cycle += 7;
        reset_pending = false;        
        return;
    }
    if(nmi_pending){
        NMI();
        //std::cout<<"NMI trigger"<<std::endl;
        nmi_pending = false;
        cpu_cycle+=7;
        return;
    }

    if(dma_pending){
        DMA();
        dma_pending = false;
        cpu_cycle += curr_cycle;
        return;
    }
    
    uint16_t old_PC = PC;
    uint8_t instr = fetch_instr();
    bool debug = false;
    
    switch(instr){
        
        // Force Interrupt Op
        IS_OP(BRK,0x00);

        
        // ALU Ops
        IS_ALU(ORA);
        IS_ALU(AND);
        IS_ALU(EOR);
        IS_ALU(ADC);
        IS_ALU(SBC);


        // RMW Ops
        IS_RMW(ASL);
        IS_RMW(ROL);
        IS_RMW(LSR);
        IS_RMW(ROR);


        // Test bits
        IS_OP_MODE(BIT,0x24,addr_mode::zp);
        IS_OP_MODE(BIT,0x2C,addr_mode::abs_);


        // Branch Ops
        IS_OP_MODE(BPL,0x10,addr_mode::rel);
        IS_OP_MODE(BMI,0x30,addr_mode::rel);
        IS_OP_MODE(BVC,0x50,addr_mode::rel);
        IS_OP_MODE(BVS,0x70,addr_mode::rel);
        IS_OP_MODE(BCC,0x90,addr_mode::rel);
        IS_OP_MODE(BCS,0xB0,addr_mode::rel);
        IS_OP_MODE(BNE,0xD0,addr_mode::rel);
        IS_OP_MODE(BEQ,0xF0,addr_mode::rel);


        // Clear Flags
        IS_OP(CLC,0x18);
        IS_OP(CLI,0x58);
        IS_OP(CLV,0xB8);
        IS_OP(CLD,0xD8);


        // Compare Ops
        IS_ALU(CMP);

        IS_OP_MODE(CPY,0xC0,addr_mode::imm);
        IS_OP_MODE(CPY,0xC4,addr_mode::zp);
        IS_OP_MODE(CPY,0xCC,addr_mode::abs_);

        IS_OP_MODE(CPX,0xE0,addr_mode::imm);
        IS_OP_MODE(CPX,0xE4,addr_mode::zp);
        IS_OP_MODE(CPX,0xEC,addr_mode::abs_);

        
        // Decrease Ops
        IS_OP_MODE(DEC,0xC6,addr_mode::zp);
        IS_OP_MODE(DEC,0xCE,addr_mode::abs_);
        IS_OP_MODE(DEC,0xD6,addr_mode::zp_x);
        IS_OP_MODE(DEC,0xDE,addr_mode::abs_x);  
        IS_OP(DEY,0x88);
        IS_OP(DEX,0xCA);


        // Increase Ops
        IS_OP_MODE(INC,0xE6,addr_mode::zp);
        IS_OP_MODE(INC,0xEE,addr_mode::abs_);
        IS_OP_MODE(INC,0xF6,addr_mode::zp_x);
        IS_OP_MODE(INC,0xFE,addr_mode::abs_x);
        IS_OP(INY,0xC8);
        IS_OP(INX,0xE8);
        
        
        // Jump Ops
        IS_OP_MODE(JSR,0x20,addr_mode::abs_);
        IS_OP_MODE(JMP,0x4C,addr_mode::abs_);
        IS_OP_MODE(JMP,0x6C,addr_mode::indir);


        // Load Ops
        IS_ALU(LDA);

        IS_OP_MODE(LDY,0xA0,addr_mode::imm);
        IS_OP_MODE(LDY,0xA4,addr_mode::zp);
        IS_OP_MODE(LDY,0xAC,addr_mode::abs_);
        IS_OP_MODE(LDY,0xB4,addr_mode::zp_x);
        IS_OP_MODE(LDY,0xBC,addr_mode::abs_x);

        IS_OP_MODE(LDX,0xA2,addr_mode::imm);
        IS_OP_MODE(LDX,0xA6,addr_mode::zp);
        IS_OP_MODE(LDX,0xAE,addr_mode::abs_);
        IS_OP_MODE(LDX,0xB6,addr_mode::zp_y);
        IS_OP_MODE(LDX,0xBE,addr_mode::abs_y);
        

        // Actual NOP
        IS_OP(NOP,0xEA);


        // Stack Ops
        IS_OP(PHP,0x08);
        IS_OP(PLP,0x28);
        IS_OP(PHA,0x48);
        IS_OP(PLA,0x68);
        
        // Return Ops
        IS_OP(RTS,0x60);
        IS_OP(RTI,0x40);
        

        // Set flags
        IS_OP(SEC,0x38);
        IS_OP(SEI,0x78);
        IS_OP(SED,0xF8);

        
        // Store Ops
        IS_ALU(STA);

        IS_OP_MODE(STY,0x84,addr_mode::zp);
        IS_OP_MODE(STY,0x8C,addr_mode::abs_);
        IS_OP_MODE(STY,0x94,addr_mode::zp_x);

        IS_OP_MODE(STX,0x86,addr_mode::zp);
        IS_OP_MODE(STX,0x8E,addr_mode::abs_);
        IS_OP_MODE(STX,0x96,addr_mode::zp_y);


        // Transfer Ops
        IS_OP(TYA,0x98);
        IS_OP(TAY,0xA8);
        IS_OP(TXA,0x8A);
        IS_OP(TXS,0x9A);
        IS_OP(TAX,0xAA);
        IS_OP(TSX,0xBA);


        // default:
        //     std::cout<<"invalid instruction: "<<std::hex<<(int)instr<<std::endl;
        //     exit(1);

    }


    cpu_cycle += curr_cycle;

    // if(old_PC == PC){
    //     std::cout<<"Entered infinite loop"<<std::endl;
    //     exit(1);
    // }
}

/*
    Execution loop
*/
// void nes_cpu::cycle(){

//     while(1)
//         execute();
    
// }

void nes_cpu::set_NMI(){
    nmi_pending = true;
}

/*
    Stores PC and status flag, sets PC to interrupt vector FFFA-FFFB
*/
void nes_cpu::NMI(){
    write(SP--,(PC & 0xFF00)>>8);
    write(SP--,(PC & 0x00FF));
    write(SP--,P ^ SET_FLAG(GET_FLAG(P,BFLAG),BFLAG));
    P ^= SET_FLAG(GET_FLAG(P,INTERR),INTERR);

    PC = read(0xFFFA) | (read(0xFFFB)<<8);
}

// void nes_cpu::set_dma_req(uint8_t val){
//     dma_req.pending = true;
//     dma_req.hi_byte = val;
// }

void nes_cpu::set_dma_req(){
    dma_pending = true;
}

void nes_cpu::DMA(){
    curr_cycle = 1;
    if(cpu_cycle % 2) curr_cycle += 1;

    uint16_t address = A<<8;

    for(int i = 0; i<256; i++){
        uint8_t val = read(address+i);
        write(0x2004,val);
    }
}

// Add with Carry
void nes_cpu::ADC(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint16_t res = A + val + GET_FLAG(P,CARRY);

    P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    if(res > 255)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(uint8_t)res);
    P ^= CALC_OVR(GET_FLAG(P,OVR),A,val,(uint8_t)res);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
    
    A = (signed char)res;

    curr_cycle += op.page_boundary;
}

// Logical AND
void nes_cpu::AND(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A &= val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += op.page_boundary;
}

// Arithmetic Shift Left
void nes_cpu::ASL(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = (uint16_t)read_op(op);

    if((val & 0x80) == 0x80) 
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);

    val <<= 1;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),val);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)val);

    if(op.kind == operand_k::addr) write(op.value, val);
    else if(op.kind == operand_k::acc) A = val; 

    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Branch if Carry Clear
void nes_cpu::BCC(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(!GET_FLAG(P,CARRY)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Carry Set
void nes_cpu::BCS(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(GET_FLAG(P,CARRY)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Equal
void nes_cpu::BEQ(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(GET_FLAG(P,ZERO)){
        PC += val;
        curr_cycle += 1;
    }
}

// Bit Test
void nes_cpu::BIT(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = val & A;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),res);
    
    if(GET_FLAG(P,OVR))
        P &= (val & 0x40) | 0xBF;
    else
        P ^= (val & 0x40);
    
    if(GET_FLAG(P,NEG))
        P &= (val & 0x80) | 0x7F;
    else
        P ^= (val & 0x80);
    
}

// Branch if Minus
void nes_cpu::BMI(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(GET_FLAG(P,NEG)){
        PC += val;
        curr_cycle += 1;
    }
}

//Branch if Not Equal
void nes_cpu::BNE(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(!GET_FLAG(P,ZERO)){
        PC += val;
        curr_cycle += 1;
    }
}

//Branch if Positive
void nes_cpu::BPL(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(!GET_FLAG(P,NEG)){
        PC += val;
        curr_cycle += 1;
    }
}

// Force Interrupt
void nes_cpu::BRK(addr_mode mode){
    write(SP--,(PC & 0xFF00)>>8);
    write(SP--,(PC & 0x00FF));
    write(SP--,P);

    PC = read(0xFFFE) | (read(0xFFFF)<<8);
    curr_cycle += 1;
}

// Branch if Overflow Clear
void nes_cpu::BVC(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(!GET_FLAG(P,OVR)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Overflow Set
void nes_cpu::BVS(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);

    if(GET_FLAG(P,OVR)){
        PC += val;
        curr_cycle += 1;
    }
}

// Clear Carry Flag
void nes_cpu::CLC(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
}

// Clear Decimal Mode
void nes_cpu::CLD(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,DECIM),DECIM);
}

// Clear Interrupt Disable
void nes_cpu::CLI(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,INTERR),INTERR);
}

// Clear Overflow Flag
void nes_cpu::CLV(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,OVR),OVR);
}

// Compare 
void nes_cpu::CMP(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = A - val;

    if(A>=val)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
    curr_cycle += op.page_boundary;
}

// Compare X Reister
void nes_cpu::CPX(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = X - val;
    if(X>=val)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
}

// Compare Y Register
void nes_cpu::CPY(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = Y - val;
    if(Y>=val)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
}

// Decremeent Memory
void nes_cpu::DEC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = (uint8_t)read_op(op);

    write(op.value, val-1);
    
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),val-1);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)(val-1));

    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Decrement X Register
void nes_cpu::DEX(addr_mode mode){
    X -= 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += 1;
}

// Decrement Y Register
void nes_cpu::DEY(addr_mode mode){
    Y -= 1;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);

    curr_cycle += 1;
}

// Exclusive OR
void nes_cpu::EOR(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = (uint8_t)read_op(op);
    A ^= val;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += op.page_boundary;
}

// Increment Memory
void nes_cpu::INC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    write(op.value, val+1);

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(uint8_t)(val+1));
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)(val+1));

    if(mode == addr_mode::abs_x) curr_cycle += 1;
    curr_cycle += 1;
}

// Increment X Register
void nes_cpu::INX(addr_mode mode){
    X += 1;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);

    curr_cycle += 1;
}

// Increment Y Register
void nes_cpu::INY(addr_mode mode){
    Y += 1;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);

    curr_cycle += 1;
}

// Jump
void nes_cpu::JMP(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    
    PC = op.value;
    curr_cycle--;
}

// Jump to Subroutine
void nes_cpu::JSR(addr_mode mode){
    operand_t op = decode_op(mode);
    uint16_t val = read_op(op);

    PC -= 1;
    write(SP--,(PC & 0xFF00)>>8);
    write(SP--,(PC & 0x00FF));
    PC = op.value;
}

// Load Accumulator
void nes_cpu::LDA(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A = val;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += op.page_boundary;
}

// Load X Register
void nes_cpu::LDX(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char) read_op(op);
    X = val;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);

    curr_cycle += op.page_boundary;
}

// Load Y Register
void nes_cpu::LDY(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = read_op(op);
    Y = val;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);

    curr_cycle += op.page_boundary;
}

// Logical Right Shift
void nes_cpu::LSR(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);

    // set carry before operations
    if(val & 1)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    
    val >>= 1;
        
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),val);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)val);
    
    if(op.kind == operand_k::addr)
        write(op.value, val);
    else if(op.kind == operand_k::acc)
        A = val; 
    
    if(mode == addr_mode::abs_x) curr_cycle += 1;
    curr_cycle +=1;
}

// No Operation
void nes_cpu::NOP(addr_mode mode){
    curr_cycle += 1;
    return;
}

// Logical Inclusive OR
void nes_cpu::ORA(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A |= val;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += op.page_boundary;
}

// Push Accumulator
void nes_cpu::PHA(addr_mode mode){
    write(SP--, A);
    curr_cycle += 1;
}

// Push Processor Status (P)
void nes_cpu::PHP(addr_mode mode){
    write(SP--, P ^ 0x10);
    //write(SP--, P | 0x10);
    curr_cycle += 1;
}

// Pull Accumulator
void nes_cpu::PLA(addr_mode mode){
    A = read(++SP);

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += 2;
}

// Pull Processor Status (P)
void nes_cpu::PLP(addr_mode mode){
    P = read(++SP) & 0xEF | 0x20;
    curr_cycle += 2;
}

// Rotate Left

void nes_cpu::ROL(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = val << 1;
    res ^= (P & 0x1);

    if((val>>7) & 1)
        SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else 
        CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),res);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);

    if(op.kind == operand_k::addr)
        write(op.value, res);   
    else if(op.kind == operand_k::acc)
        A = res;  
    
    if(mode == addr_mode::abs_x) curr_cycle += 1;
    curr_cycle += 1;
}

// Rotate Right
void nes_cpu::ROR(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = val >> 1;
    res |= ((P & 1)<<7);

    if(val & 1)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),res);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);

    if(op.kind == operand_k::addr)
        write(op.value, res);
    else if(op.kind == operand_k::acc)
        A = res;
    
    if(mode == addr_mode::abs_x) curr_cycle += 1;
    curr_cycle += 1;
}

// Return from Interrupt
void nes_cpu::RTI(addr_mode mode){
    P = read(++SP) & 0xEF | 0x20;
    uint8_t low = read(++SP);
    uint8_t high = read(++SP);
    PC = (high<<8) | (low);

    curr_cycle += 2;
}

// Return from Subroutine
void nes_cpu::RTS(addr_mode mode){
    uint8_t low = read(++SP);
    uint8_t high = read(++SP);
    PC = ((high<<8) | (low)) + 1;

    curr_cycle += 3;
}

// Subtract with Carry
void nes_cpu::SBC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = ~(read_op(op));
    signed short res = A + val + GET_FLAG(P,CARRY);

    P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    if(res > 255)
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    P ^= CALC_OVR(GET_FLAG(P,OVR),A,val,(uint8_t)res);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(uint8_t)res);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);

    A = (uint8_t)res;
    curr_cycle += op.page_boundary;
}

// Set Carry Flag
void nes_cpu::SEC(addr_mode mode){
    P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    curr_cycle += 1;
}

// Set Decimal Flag
void nes_cpu::SED(addr_mode mode){
    P ^= SET_FLAG(GET_FLAG(P,DECIM),DECIM);
    curr_cycle += 1;
}

// Set Interrupt Flag
void nes_cpu::SEI(addr_mode mode){
    P ^= SET_FLAG(GET_FLAG(P,INTERR),INTERR);
    curr_cycle += 1;
}

// Store Accumulator
void nes_cpu::STA(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, A);
    if((mode == addr_mode::indir_y || mode == addr_mode::abs_y || mode == addr_mode::abs_x)) curr_cycle += 1;
}

// Store X Register
void nes_cpu::STX(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, X);
}

// Store Y Register
void nes_cpu::STY(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, Y);
}

// Transfer Accumulator to X
void nes_cpu::TAX(addr_mode mode){
    X = A;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);

    curr_cycle += 1;
}

// Transfer Accumulator to Y
void nes_cpu::TAY(addr_mode mode){
    Y = A;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);

    curr_cycle += 1;
}

// Transfer Stack Pointer to X
void nes_cpu::TSX(addr_mode mode){
    X = SP;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);

    curr_cycle += 1;
}

// Transfer X to Accumulator
void nes_cpu::TXA(addr_mode mode){
    A = X;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += 1;
}

// Transfer X to Stack Pointer
void nes_cpu::TXS(addr_mode mode){
    SP = X + STACK_LEN;
    curr_cycle += 1;
}

// Transfer Y to Accumulator
void nes_cpu::TYA(addr_mode mode){
    A = Y;

    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += 1;
}

//helper functions for addresses
uint16_t nes_cpu::zp_indexed(uint16_t arg, uint8_t reg){
    return (arg + reg) % 256;
}

uint16_t nes_cpu::abs_indexed(uint16_t arg, uint8_t reg){
    return (arg + reg);
}

uint16_t nes_cpu::indirect(uint16_t arg){
    uint16_t res = read(arg);
    if((arg & 0x00FF) == 0x00FF) res |= (read(arg&0xFF00) << 8);
    else res |= (read(arg+1)<<8);
    return res;
}

uint16_t nes_cpu::dx(uint16_t arg){
    return read((arg + X) % 265) + read(((arg + X + 1) % 256)) * 256;
}

uint16_t nes_cpu::dy(uint16_t arg){
    return read(arg) + read((arg + 1) % 256) * 256 + Y;
}

/*
    Prints out current execution and state of CPU
*/
void nes_cpu::op_trace(std::string op, addr_mode mode){
    
    std::cout<<std::uppercase<<std::hex<<std::setw(4)<<(int)(PC-1)<<"  "<<std::hex<<std::setw(2)<<(int)_mem->PEEK(PC-1)<<" ";
    operand_t operand = decode_op(mode,true);
    uint16_t val = read_op(operand); 
    std::cout<<op<<" ";
    switch(mode){
        case addr_mode::accum:
            std::cout<<"A\t\t\t\t\t";
            break;

        case addr_mode::impl:
            std::cout<<"\t\t\t\t\t";
            break;

        case addr_mode::imm:
            std::cout<<"#"<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<operand.value<<"\t\t\t\t";

            break;
        case addr_mode::abs_:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(4)<<operand.value<<"";
            if(op!="JMP" && op!="JSR") std::cout<<"= "<<std::uppercase<<std::hex<<std::setw(2)<<(int)val<<"\t\t\t";
            else std::cout<<"\t\t\t\t";
            break;

        case addr_mode::abs_x:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(4)<<(int)operand.trace<<",X @ "<<
                std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::abs_y:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(4)<<(int)operand.trace<<",Y @ "<<
                std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::zp:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.value<<" = "
                <<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::zp_x:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.trace<<", X @ "
                <<std::setw(2)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t";
            break;
        case addr_mode::zp_y:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.trace<<", Y @ "
                <<std::setw(2)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t";
            break;
        
        case addr_mode::indir:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(4)<<(int)operand.trace<<") = "
                <<std::setw(4)<<(int)operand.value<<"\t\t";
            break;

        case addr_mode::indir_x:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(2)<<(int)operand.trace<<",X) @ "
                <<std::setw(2)<<(int)((operand.trace+X)%256)<<" = "<<std::setw(4)<<(int)operand.value
                <<" = "<<std::setw(2)<<(int)val<<"\t";
            break;
        
        case addr_mode::indir_y:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(2)<<(int)operand.trace<<"),Y = "
                <<std::setw(4)<<(uint16_t)(operand.value-Y)<<" @ "<<std::setw(4)<<(int)operand.value
                <<" = "<<std::setw(2)<<(int)val<<"\t";
            break;
        
        case addr_mode::rel:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<PC+1+val<<"\t\t\t\t";
            break;
        
    }

    std::cout<<std::uppercase<<std::hex<<"A:"<<std::setw(2)<<(int)A<<" "<<"X:"<<std::setw(2)<<(int)X
        <<" "<<"Y:"<<std::setw(2)<<(int)Y<<" "<<"P:"<<std::setw(2)<<(int)P<<" "<<"SP:"<<(int)(SP-STACK_LEN)
        <<std::dec<<" CYC:"<<cpu_cycle<<std::endl;
    
    curr_cycle = 1;
    
}



