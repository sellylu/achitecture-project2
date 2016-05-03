//
// Created by selly on 2016/4/26.
//

#include "simulator.h"

bool Simulator::checkError(int* sum1, int* sum2) {
    bool any = false;
    brk = false;
    // write to $0
    if (ps.wbs->prev.write_back && ps.wbs->prev.rd == 0 &&
            !(ps.wbs->prev.op == R_FORMAT && ps.wbs->prev.func == FUNC_SLL)){
        brk = true;
        writeError(WriteRegZero);
    }
    // address overflow
    if(checkAddrOverflow(ps.mems->prev.alu_result, ps.mems->prev.op)) {
        halt = true;
        any = true;
    }
    // data misaligned
    if(checkDataMisaligned(ps.mems->prev.alu_result, ps.mems->prev.op)) {
        halt = true;
        any = true;
    }

    // number overflow
    if(ps.exs->prev.op == R_FORMAT && ps.exs->prev.func == FUNC_ADD) {
        checkNumOverflow(ps.exs->prev.rs_val, ps.exs->prev.rt_val, sum1);
    } else if(ps.exs->prev.op == R_FORMAT && ps.exs->prev.func == FUNC_SUB) {
        checkNumOverflow(ps.exs->prev.rs_val, -ps.exs->prev.rt_val, sum1);
    } else if((ps.exs->prev.op == OP_ADDI) ||
            (ps.exs->prev.op >= OP_LB && ps.exs->prev.op <= OP_SW)) {
        checkNumOverflow(ps.exs->prev.rs_val, ps.exs->prev.other, sum1);
    }
    Instruction instr(ps.ids->prev.ori);
    instr.decode();
    checkNumOverflow(nowPC, IndexToAddr(instr.other), sum2);
    if(any)
        registers[PCReg] = nowPC;
    return any;
}

void Simulator::checkNumOverflow(int a, int b, int *sum) {
    *sum = a + b;
    if (!((a ^ b) & SIGN_BIT) && ((a ^ *sum) & SIGN_BIT)) {
        writeError(NumOverflow);
    }
}
bool Simulator::checkAddrOverflow(int sum, int op) {
    int length = 1;
    switch (op) {
        case OP_LW:
        case OP_SW:
            break;
        case OP_LH:
        case OP_LHU:
        case OP_SH:
            length = 2;
            break;
        case OP_LB:
        case OP_LBU:
        case OP_SB:
            length = 4;
            break;
    }
    if((sum >= MAX_MEMORY || (sum + WORD/length) > MAX_MEMORY || sum < MIN_MEMORY) &&
       (op >= OP_LB && op <= OP_SW)) {
        writeError(AddrOverflow);
        return true;
    }
    return false;
}
bool Simulator::checkDataMisaligned(int sum, int op) {
    switch (op) {
        case OP_LW:
        case OP_SW:
            if(sum % WORD != 0) {
                writeError(DataMisaligned);
                return true;
            }
            break;
        case OP_LH:
        case OP_LHU:
        case OP_SH:
            if(sum % WORD != 0 && sum % WORD != 2) {
                writeError(DataMisaligned);
                return true;
            }
            break;
    }
    return false;
}

void Simulator::writeError(int type) {
    error.open(error_path, std::ios::out | std::ios::app);

    error << "In cycle " << cycle+1 << ": ";
    switch(type) {
        case WriteRegZero:
            error << "Write $0 Error" << std::endl;
            break;
        case NumOverflow:
            error << "Number Overflow" << std::endl;
            break;
        case AddrOverflow:
            error << "Address Overflow" << std::endl;
            break;
        case DataMisaligned:
            error << "Misalignment Error" << std::endl;
            break;
        default:
            error << "Something Wrong" << std::endl;
    }
    error.close();
}
