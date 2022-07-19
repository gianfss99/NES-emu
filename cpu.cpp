#include "cpu.hpp"
#include "helper.hpp"

#define ALU_OP(op,offset,mode)                              \
            case alu_op_codes::op##_base + offset:          \
                trace(#op,mode);                            \
                op(mode);                                   \
                break;

#define IS_ALU(op) \
            ALU_OP(op,0x01,addr_mode::indir_x);  \
            ALU_OP(op,0x05,addr_mode::zp);       \
            ALU_OP(op,0x09,addr_mode::imm);      \
            ALU_OP(op,0x0D,addr_mode::abs_);     \
            ALU_OP(op,0x11,addr_mode::indir_y);  \
            ALU_OP(op,0x15,addr_mode::zp_x);     \
            ALU_OP(op,0x19,addr_mode::abs_y);    \
            ALU_OP(op,0x1D,addr_mode::abs_x);

#define RMW_OP(op,offset,mode)  \
            case rmw_op_codes::op##_base + offset:  \
                trace(#op,mode);\
                op(mode);                           \
                break;                 

#define IS_RMW(op)   \
            RMW_OP(op,0x06,addr_mode::zp);       \
            RMW_OP(op,0x0A,addr_mode::accum);      \
            RMW_OP(op,0x0E,addr_mode::abs_);      \
            RMW_OP(op,0x16,addr_mode::zp_x);     \
            RMW_OP(op,0x1E,addr_mode::abs_x);

#define IS_OP_MODE(op,opcode,mode) case opcode: \
                                        trace(#op,mode);\
                                        op(mode); \
                                        break;

#define IS_OP(op,opcode) case opcode:\
                            trace(#op,addr_mode::impl);\
                            op(addr_mode::impl);\
                            break;

Cpu::Cpu(){
    A = 0;
    X = 0;
    Y = 0;
    P = 0x24;
    PC = PC_START;
    S = STACK_START;
    cpu_cycle = 7;
    curr_cycle = 0;
    _mem = Memory();
    _mem.mount(CPU_ADDR_SPACE);
}

uint8_t Cpu::read(uint16_t i){
    curr_cycle += 1;
    return _mem.PEEK(i);
}

void Cpu::write(uint16_t i, uint8_t val){
    curr_cycle += 1;
    _mem.SET(i,val);
}

// void Cpu::add_cycles(addr_mode mode, bool pg_crossed){
//     switch(mode){
//         case addr_mode::accum:
//             cpu_cycle += 2;
//             break;

//         case addr_mode::impl:
//             break;

//         case addr_mode::imm:
//             break;

        

//         case addr_mode::zp:
//             cycle_no += 5;
//             break;

//         case addr_mode::zp_x:
//         case addr_mode::zp_y:
//         case addr_mode::abs_:
//             cycle_no += 6;
//             break;

//         case addr_mode::abs_x:
//         case addr_mode::abs_y:
//             cycle_no += 7;
//             break;
        
//         case addr_mode::indir:
//             break;

//         case addr_mode::indir_x:
//             break;
        
//         case addr_mode::indir_y:
//             break;
        
//         case addr_mode::rel:
//             break; 
//     }
// }

void Cpu::trace(std::string op, addr_mode mode){
    
    std::cout<<std::uppercase<<std::hex<<std::setw(4)<<(int)(PC-1)<<"  "<<std::hex<<std::setw(2)<<(int)_mem.PEEK(PC-1)<<" ";
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
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(4)<<(int)operand.trace<<",X @ "<<std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::abs_y:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(4)<<(int)operand.trace<<",Y @ "<<std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::zp:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t\t";
            break;

        case addr_mode::zp_x:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.trace<<", X @ "<<std::setw(2)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t";
            break;
        case addr_mode::zp_y:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<(int)operand.trace<<", Y @ "<<std::setw(2)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t\t";
            break;
        
        case addr_mode::indir:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(4)<<(int)operand.trace<<") = "<<std::setw(4)<<(int)operand.value<<"\t\t";
            break;

        case addr_mode::indir_x:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(2)<<(int)operand.trace<<",X) @ "
                <<std::setw(2)<<(int)((operand.trace+X)%256)<<" = "<<std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t";
            break;
        
        case addr_mode::indir_y:
            std::cout<<std::uppercase<<std::hex<<"($"<<std::setw(2)<<(int)operand.trace<<"),Y = "
                <<std::setw(4)<<(uint16_t)(operand.value-Y)<<" @ "<<std::setw(4)<<(int)operand.value<<" = "<<std::setw(2)<<(int)val<<"\t";
            break;
        
        case addr_mode::rel:
            std::cout<<std::uppercase<<std::hex<<"$"<<std::setw(2)<<PC+1+val<<"\t\t\t\t";
            break;
        
    }

    std::cout<<std::uppercase<<std::hex<<"A:"<<std::setw(2)<<(int)A<<" "<<"X:"<<std::setw(2)<<(int)X<<" "<<"Y:"
        <<std::setw(2)<<(int)Y<<" "<<"P:"<<std::setw(2)<<(int)P<<" "<<"SP:"<<(int)(S-STACK_LEN)<<std::dec<<" CYC:"<<cpu_cycle<<std::endl;
    curr_cycle = 1;
    
}

operand_t Cpu::decode_op(addr_mode mode, bool trace/*=false*/){

    operand_t operand;
    operand.page_boundary = false;
    switch(mode){
        case addr_mode::accum:
        case addr_mode::impl:
            operand.kind = operand_k::acc;
            if(trace){
                std::cout<<"       ";
            }
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

uint8_t Cpu::read_op(operand_t operand){

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

void Cpu::execute(){
    uint16_t old_PC = PC;
    curr_cycle = 0;
    // std::cout<<"Curr PC = "<<std::hex<<(int)PC<<std::endl;
    uint8_t instr = fetch_instr();
    
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


        default:
            std::cout<<"invalid instruction: "<<std::hex<<(int)instr<<std::endl;
            exit(1);

    }


    cpu_cycle += curr_cycle;

    if(old_PC == PC){
        std::cout<<"Entered infinite loop"<<std::endl;
        exit(1);
    }
}

void Cpu::cycle(){

    while(1){
        //std::cout<<std::hex<<"Curr PC: "<<(int)(PC)<<std::endl;
        execute();
        // std::cout<<std::dec<<"A="<<(int)A<<std::endl;
        // std::cout<<std::dec<<"X="<<(int)X<<std::endl;
        // std::cout<<std::dec<<"Y="<<(int)Y<<std::endl;
        // std::bitset<8> bin_P(P);
        // std::cout<<"P="<<bin_P<<std::endl;
        // std::cout<<"P=NV-BDIZC"<<std::endl;
        // std::cout<<std::hex<<"S="<<(int)S<<std::endl;
        // std::cout<<"----------------------------"<<std::endl;
    }
}

//Add with Carry
void Cpu::ADC(addr_mode mode){

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

// Binary AND
void Cpu::AND(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A &= val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);

    curr_cycle += op.page_boundary;
}

// Arithmetic Shift Left
void Cpu::ASL(addr_mode mode){

    operand_t op = decode_op(mode);
    uint8_t val = (uint16_t)read_op(op);

    if((val & 0x80) == 0x80) 
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    else
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);

    val <<= 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),val);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)val);

    if(op.kind == operand_k::addr){
        write(op.value, val);
    }

    else if(op.kind == operand_k::acc){
        A = val; 
    }
    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Branch if Carry Clear
void Cpu::BCC(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(!GET_FLAG(P,CARRY)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Carry Set
void Cpu::BCS(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(GET_FLAG(P,CARRY)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Equal
void Cpu::BEQ(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(GET_FLAG(P,ZERO)){
        if((PC-2) == PC+val){
            //std::cout<<"Infinite loop at addr: "<<std::hex<<(int)op.value<<std::endl;
            exit(1);
        }
        PC += val;
        curr_cycle += 1;
    }
}

// Bit Test
void Cpu::BIT(addr_mode mode){
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
void Cpu::BMI(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(GET_FLAG(P,NEG)){
        PC += val;
        curr_cycle += 1;
    }
}

//Branch if Not Equal
void Cpu::BNE(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
   // std::cout<<std::hex<<"BNE val: "<<(int)val<<std::endl;
    if(!GET_FLAG(P,ZERO)){
        PC += val;
        curr_cycle += 1;
    }
}

//Branch if Positive
void Cpu::BPL(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(!GET_FLAG(P,NEG)){
        PC += val;
        curr_cycle += 1;
    }
}

// Force Interrupt
void Cpu::BRK(addr_mode mode){
    write(S--,(PC & 0xFF00)>>8);
    write(S--,(PC & 0x00FF));
    
    write(S--,P);
    // P |= (1<<4);

    PC = read(0xFFFE) | (read(0xFFFF)<<8);
    curr_cycle += 1;
    //std::cout<<std::hex<<"BRK...: "<<(int)read(0xFFFF)<<std::endl;
}

// Branch if Overflow Clear
void Cpu::BVC(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(!GET_FLAG(P,OVR)){
        PC += val;
        curr_cycle += 1;
    }
}

// Branch if Overflow Set
void Cpu::BVS(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char)read_op(op);
    if(GET_FLAG(P,OVR)){
        PC += val;
        curr_cycle += 1;
    }
}

// Clear Carry Flag
void Cpu::CLC(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
}

// Clear Decimal Mode
void Cpu::CLD(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,DECIM),DECIM);
}

// Clear Interrupt Disable
void Cpu::CLI(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,INTERR),INTERR);
}

// Clear Overflow Flag
void Cpu::CLV(addr_mode mode){
    curr_cycle += 1;
    P ^= CLR_FLAG(GET_FLAG(P,OVR),OVR);
}

// Compare 
void Cpu::CMP(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = A - val;
    if(A>=val){
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    }else{
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    }
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
    curr_cycle += op.page_boundary;
}

// Compare X Reister
void Cpu::CPX(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = X - val;
    if(X>=val){
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    }else{
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    }
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
}

// Compare Y Register
void Cpu::CPY(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    uint8_t res = Y - val;
    if(Y>=val){
        P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
    }else{
        P ^= CLR_FLAG(GET_FLAG(P,CARRY),CARRY);
    }
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(signed char)res);
    P^= CALC_NEG(GET_FLAG(P,NEG),(signed char)res);
}

// Decremeent Memory
void Cpu::DEC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = (uint8_t)read_op(op);

    write(op.value, val-1);
    
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),val-1);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)(val-1));

    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Decrement X Register
void Cpu::DEX(addr_mode mode){
    X -= 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += 1;
}

// Decrement Y Register
void Cpu::DEY(addr_mode mode){
    Y -= 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);
    curr_cycle += 1;
}

// Exclusive OR
void Cpu::EOR(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = (uint8_t)read_op(op);
    A ^= val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += op.page_boundary;
}

// Increment Memory
void Cpu::INC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);

    write(op.value, val+1);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),(uint8_t)(val+1));
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)(val+1));
    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Increment X Register
void Cpu::INX(addr_mode mode){
    X += 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += 1;
}

// Increment Y Register
void Cpu::INY(addr_mode mode){
    Y += 1;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);
    curr_cycle += 1;
}

// Jump
void Cpu::JMP(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    curr_cycle--;
    //std::cout<<std::hex<<"Jump Val: "<<(int)op.value<<std::endl;
    if((PC-3) == op.value){
        //std::cout<<"Infinite loop at addr: "<<std::hex<<(int)op.value<<std::endl;
        exit(1);
    }
    PC = op.value;
}

// Jump to Subroutine
void Cpu::JSR(addr_mode mode){
    operand_t op = decode_op(mode);
    uint16_t val = read_op(op);
    PC -= 1;
    write(S--,(PC & 0xFF00)>>8);
    write(S--,(PC & 0x00FF));
    PC = op.value;
}

// Load Accumulator
void Cpu::LDA(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A = val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += op.page_boundary;
}

// Load X Register
void Cpu::LDX(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = (signed char) read_op(op);
    X = val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += op.page_boundary;
}

// Load Y Register
void Cpu::LDY(addr_mode mode){
    operand_t op = decode_op(mode);
    signed char val = read_op(op);
    Y = val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);
    curr_cycle += op.page_boundary;
}

// Logical Right Shift
void Cpu::LSR(addr_mode mode){

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
    
    if(op.kind == operand_k::addr){
        write(op.value, val);
    }

    else if(op.kind == operand_k::acc){
        A = val; 
    }
    curr_cycle +=1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// No Operation
void Cpu::NOP(addr_mode mode){
    curr_cycle += 1;
    return;
}

// Logical Inclusive OR
void Cpu::ORA(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = read_op(op);
    A |= val;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += op.page_boundary;
}

// Push Accumulator
void Cpu::PHA(addr_mode mode){
    write(S--, A);
    curr_cycle += 1;
}

// Push Processor Status (P)
void Cpu::PHP(addr_mode mode){
    write(S--, P ^ 0x10);
    curr_cycle += 1;
}

// Pull Accumulator
void Cpu::PLA(addr_mode mode){
    A = read(++S);
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += 2;
}

// Pull Processor Status (P)
void Cpu::PLP(addr_mode mode){
    P = read(++S) & 0xEF | 0x20;
    curr_cycle += 2;
}

// Rotate Left

void Cpu::ROL(addr_mode mode){
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

    if(op.kind == operand_k::addr){
        write(op.value, res);
    }
        
    else if(op.kind == operand_k::acc){
        A = res;
        
    }
    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
}

// Rotate Right
void Cpu::ROR(addr_mode mode){
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

    if(op.kind == operand_k::addr){
        write(op.value, res);
        
    }
    else if(op.kind == operand_k::acc){
        A = res;
        
    }
    curr_cycle += 1;
    if(mode == addr_mode::abs_x) curr_cycle += 1;
    
}

// Return from Interrupt
void Cpu::RTI(addr_mode mode){
    P = read(++S) & 0xEF | 0x20;
    uint8_t low = read(++S);
    uint8_t high = read(++S);
    PC = (high<<8) | (low);
    curr_cycle += 2;
    //std::cout<<"RTI..."<<std::endl;
}

// Return from Subroutine
void Cpu::RTS(addr_mode mode){
    uint8_t low = read(++S);
    uint8_t high = read(++S);
    PC = ((high<<8) | (low)) + 1;
    curr_cycle += 3;
}

// Subtract with Carry
void Cpu::SBC(addr_mode mode){
    operand_t op = decode_op(mode);
    uint8_t val = ~(read_op(op));
    signed short res = A + val + GET_FLAG(P,CARRY);
    uint8_t res_8 = A + val + GET_FLAG(P,CARRY);
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
void Cpu::SEC(addr_mode mode){
    curr_cycle += 1;
    P ^= SET_FLAG(GET_FLAG(P,CARRY),CARRY);
}

// Set Decimal Flag
void Cpu::SED(addr_mode mode){
    curr_cycle += 1;
    P ^= SET_FLAG(GET_FLAG(P,DECIM),DECIM);
}

// Set Interrupt Flag
void Cpu::SEI(addr_mode mode){
    curr_cycle += 1;
    P ^= SET_FLAG(GET_FLAG(P,INTERR),INTERR);
}

// Store Accumulator
void Cpu::STA(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, A);
    if((mode == addr_mode::indir_y || mode == addr_mode::abs_y || mode == addr_mode::abs_x)) curr_cycle += 1;
}

// Store X Register
void Cpu::STX(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, X);
}

// Store Y Register
void Cpu::STY(addr_mode mode){
    operand_t op = decode_op(mode);
    write(op.value, Y);
}

// Transfer Accumulator to X
void Cpu::TAX(addr_mode mode){
    X = A;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += 1;
}

// Transfer Accumulator to Y
void Cpu::TAY(addr_mode mode){
    Y = A;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),Y);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)Y);
    curr_cycle += 1;
}

// Transfer Stack Pointer to X
void Cpu::TSX(addr_mode mode){
    X = S;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),X);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)X);
    curr_cycle += 1;
}

// Transfer X to Accumulator
void Cpu::TXA(addr_mode mode){
    A = X;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += 1;
}

// Transfer X to Stack Pointer
void Cpu::TXS(addr_mode mode){
    S = X+STACK_LEN;
    curr_cycle += 1;
}

// Transfer Y to Accumulator
void Cpu::TYA(addr_mode mode){
    A = Y;
    P ^= CALC_ZERO(GET_FLAG(P,ZERO),A);
    P ^= CALC_NEG(GET_FLAG(P,NEG),(signed char)A);
    curr_cycle += 1;
}

uint8_t Cpu::fetch_instr(){
    return read(PC++);
}

uint16_t Cpu::fetch_op(int bytes){
    uint16_t result = 0;
    for(int i = 0; i<bytes; i++){
        //std::cout<<std::hex<<"byte "<<(int)read(PC)<<std::endl;
        result |= (read(PC++) << (8*i));
    }
    return result;
}

//helper functions for addresses
uint16_t Cpu::zp_indexed(uint16_t arg, uint8_t reg){
    return (arg + reg) % 256;
}

uint16_t Cpu::abs_indexed(uint16_t arg, uint8_t reg){
    return (arg + reg);
}

uint16_t Cpu::indirect(uint16_t arg){
    uint16_t res = read(arg);
    if((arg & 0x00FF) == 0x00FF) res |= (read(arg&0xFF00) << 8);
    else res |= (read(arg+1)<<8);
    return res;
}

uint16_t Cpu::dx(uint16_t arg){
    return read((arg + X) % 265) + read(((arg + X + 1) % 256)) * 256;
}

uint16_t Cpu::dy(uint16_t arg){
    return read(arg) + read((arg + 1) % 256) * 256 + Y;
}



int Cpu::test_file(char* filename){
    FILE *fileptr;
    int filelen;
    
    fileptr = fopen(filename, "r");

    fseek(fileptr,0,SEEK_END);
    filelen = ftell(fileptr);
    //std::cout<<"filelen: "<<filelen<<std::endl;
    //if(filelen>MAX_PROGRAM_SIZE) return -1;
    rewind(fileptr);
    
    for(int i = PROGRAM_OFFSET; i<(filelen+PROGRAM_OFFSET); i++){
        _mem.SET(i, (uint8_t) fgetc(fileptr));
        //printf("%X\n",buf[i]);
    }
    
    return 1;
}  



