#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <cstdio>

#define CARRY 0
#define ZERO 1
#define INTERR 2
#define DECIM 3
#define BREAK 4
#define OVR 6
#define NEG 7

#define GET_FLAG(P,n) ((P & (1<<n))>>n)
#define SET_FLAG(flag,n) (flag==1) ? 0 : (1<<n)
#define CLR_FLAG(flag,n) (flag==1) ? (1<<n) : 0

#define CALC_CARRY(flag,val) (((val&0x100)>>8) ? (1 - flag) : (flag))
#define CALC_ZERO(flag,val) ((val==0) ? ((1 - flag)<<1) : (flag<<1))
#define CALC_OVR(flag,a,operand,res) (((((~a^operand)) & (a ^ res) & 0x80)== 0x80) ? ((1 - flag)<<6) : (flag<<6))
#define CALC_NEG(flag,val) ((val<0) ? ((1-flag)<<7) : (flag<<7))

#define ALU_OP(op,offset,mode)                              \
            case alu_op_codes::op##_base + offset:          \
                op_trace(#op,mode);                            \
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
                op_trace(#op,mode);\
                op(mode);                           \
                break;                 

#define IS_RMW(op)   \
            RMW_OP(op,0x06,addr_mode::zp);       \
            RMW_OP(op,0x0A,addr_mode::accum);      \
            RMW_OP(op,0x0E,addr_mode::abs_);      \
            RMW_OP(op,0x16,addr_mode::zp_x);     \
            RMW_OP(op,0x1E,addr_mode::abs_x);

#define IS_OP_MODE(op,opcode,mode) case opcode: \
                                        op_trace(#op,mode);\
                                        op(mode); \
                                        break;

#define IS_OP(op,opcode) case opcode:\
                            op_trace(#op,addr_mode::impl);\
                            op(addr_mode::impl);\
                            break;

int get_file_size(FILE* fileptr);

#endif