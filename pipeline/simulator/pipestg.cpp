//
// Created by selly on 2016/4/26.
//

#include <sstream>
#include <iomanip>
#include "pipestg.h"
#include "simulator.h"
//extern void writeReg();

PipeStg::PipeStg(Simulator* s) {
    this->sim = s;
    ifs = new IFstage(s);
    ids = new IDstage(s);
    exs = new EXstage(s);
    mems = new MEMstage(s);
    wbs = new WBstage(s);
}

void PipeStg::IFstage::execute() {
    sim->registers[PCReg] = sim->nowPC;
    sim->nowPC += WORD;
    next.PC = sim->nowPC;

    int index = (sim->registers[PCReg] - sim->PC_init)/WORD;
    if(index < 0) {
        next.ori = 0;
        prev_name = "NOP";
    } else {
        next.ori = sim->raw_instr[index];

        std::stringstream ss;
        ss << std::setw(8) << std::setfill('0') << std::hex << std::uppercase << sim->raw_instr[index];
        prev_name = ss.str();
    }
    if(next.ori == 0xffffffff) {
        next.halt = true;
        return;
    } else {
        next.halt = false;
    }
}
void PipeStg::IDstage::execute() {
    // Set ID/EX register
    Instruction instr(prev.ori);
    next.copyInstr(instr.decode());
    next.PC = prev.PC;
    next.rs_val = sim->registers[instr.rs];
    next.rt_val = sim->registers[instr.rt];
    next.mem_rw = false;
    next.write_back = false;
    if(instr.op >= 0x20 && instr.op <= 0x2b)
        next.mem_rw = true;
    if(!((instr.op >= OP_SB && instr.op <= OP_SW) ||
            (instr.op >= OP_BEQ && instr.op <= OP_BGTZ) ||
            (instr.op == R_FORMAT && instr.func == FUNC_JR) ||
            instr.op == OP_J || instr.op == OP_HALT))
        next.write_back = true;

    prev_name = getInstrName(instr.op, instr.func);
    if(instr.ori == 0)
        prev_name = "NOP";

    // if halt 0x3f
    if(next.op == 0x3f) {
        next.halt = true;
        return;
    } else {
        next.halt = false;
    }

    if((sim->ps.exs->next.op >= OP_LB && sim->ps.exs->next.op <= OP_LHU)) {
        if(sim->ps.exs->next.rd == instr.rs) {
            if((instr.op == R_FORMAT && instr.func > FUNC_SRA) ||
               (instr.op != R_FORMAT && instr.op != OP_LUI && instr.op != OP_J && instr.op != OP_JAL && instr.op != OP_HALT)) {
                doNOP();
                sim->stall = true;
            }
        } else if(sim->ps.exs->next.rd == instr.rt) {
            if((instr.op == R_FORMAT && instr.func != FUNC_JR) ||
               (instr.op != R_FORMAT && ((instr.op >= OP_SB && instr.op <= OP_SW) || instr.op == OP_BEQ || instr.op == OP_BNE))) {
                doNOP();
                sim->stall = true;
            }
        }
    }
    if(sim->ps.mems->next.rd != 0) {
        if(sim->ps.mems->next.rd == instr.rs && sim->ps.exs->next.rd != instr.rs) {
            if((instr.op == R_FORMAT && instr.func > FUNC_SRA) ||
               (instr.op != R_FORMAT && instr.op != OP_LUI && instr.op != OP_J && instr.op != OP_JAL && instr.op != OP_HALT)) {
                if(instr.op >= OP_BEQ && instr.op <= OP_BGTZ && sim->ps.wbs->prev.op >= OP_LB && sim->ps.wbs->prev.op <= OP_LHU) {
                    sim->forwardID = true;
                } else {
                    doNOP();
                    sim->stall = true;
                }
            }
        } else if(sim->ps.mems->next.rd == instr.rt && sim->ps.exs->next.rd != instr.rt) {
            if((instr.op == R_FORMAT && instr.func != FUNC_JR) ||
               (instr.op != R_FORMAT && ((instr.op >= OP_SB && instr.op <= OP_SW) || instr.op == OP_BEQ || instr.op == OP_BNE))) {
                if(instr.op >= OP_BEQ && instr.op <= OP_BGTZ && sim->ps.wbs->prev.op >= OP_LB && sim->ps.wbs->prev.op <= OP_LHU) {
                    sim->forwardID = true;
                } else {
                    doNOP();
                    sim->stall = true;
                }
            }
        }
    }
    // IF.Flush
    if (((next.rs_val == next.rt_val && instr.op == OP_BEQ) ||
         (next.rs_val != next.rt_val && instr.op == OP_BNE) ||
         (next.rs_val > 0 && instr.op == OP_BGTZ)) && !(sim->stall || sim->forwardID)) {
        sim->flush = true;
    }

}
void PipeStg::EXstage::execute(int sum) {
    // Set EX/MEM register
    next.op = prev.op;
    next.func = prev.func;
    next.write_back = prev.write_back;
    next.mem_rw = prev.mem_rw;
    if(prev.op == R_FORMAT)
        next.rd = prev.rd;
    else
        next.rd = prev.rt;

    // if halt 0x3f
    if(next.op == 0x3f) {
        next.halt = true;
        return;
    } else {
        next.halt = false;
    }

    unsigned int uint;
    // do execute
    switch (prev.op) {
        case R_FORMAT:

            switch (prev.func) {
                case FUNC_ADD:
                    next.alu_result = sum;
                    break;
                case FUNC_ADDU:
                    next.alu_result = prev.rs_val + prev.rt_val;
                    break;
                case FUNC_SUB:
                    next.alu_result = sum;
                    break;
                case FUNC_AND:
                    next.alu_result = prev.rs_val & prev.rt_val;
                    break;
                case FUNC_OR:
                    next.alu_result = prev.rs_val | prev.rt_val;
                    break;
                case FUNC_XOR:
                    next.alu_result = prev.rs_val ^ prev.rt_val;
                    break;
                case FUNC_NOR:
                    next.alu_result = ~(prev.rs_val | prev.rt_val);
                    break;
                case FUNC_NAND:
                    next.alu_result = ~(prev.rs_val & prev.rt_val);
                    break;
                case FUNC_SLT:
                    if (prev.rs_val < prev.rt_val)
                        next.alu_result = 1;
                    else
                        next.alu_result = 0;
                    break;
                case FUNC_SLL:
                    next.alu_result = prev.rt_val << (prev.other >> 6);
                    break;
                case FUNC_SRL:
                    uint = (unsigned int) prev.rt_val;
                    uint >>= prev.other >> 6;
                    next.alu_result = uint;
                    break;
                case FUNC_SRA:
                    next.alu_result = prev.rt_val >> (prev.other >> 6);
                    break;
                case FUNC_JR:
                    sim->nowPC = prev.rs_val;
                    break;
            }
            break;
        case OP_ADDI:
            next.alu_result = sum;
            break;
        case OP_ADDIU:
            next.alu_result = prev.rs_val + prev.other;
            break;
        case OP_LW:
        case OP_LH:
        case OP_LHU:
        case OP_LB:
        case OP_LBU:
        case OP_SW:
        case OP_SH:
        case OP_SB:
            next.alu_result = sum;
            break;
        case OP_LUI:
            next.alu_result = prev.other << 16;
            break;
        case OP_ANDI:
            next.alu_result = prev.rs_val & (prev.other & 0xffff);
            break;
        case OP_ORI:
            next.alu_result = prev.rs_val | (prev.other & 0xffff);
            break;
        case OP_NORI:
            next.alu_result = ~(prev.rs_val | (prev.other & 0xffff));
            break;
        case OP_SLTI:
            if (prev.rs_val < prev.other)
                next.alu_result = 1;
            else
                next.alu_result = 0;
            break;
        case OP_J:
            sim->nowPC = (sim->nowPC & 0xf0000000) | IndexToAddr(prev.other);
            break;
        case OP_JAL:
            next.rd = ReturnAddrReg;
            next.alu_result = sim->nowPC;
            sim->nowPC = (sim->nowPC & 0xf0000000) | IndexToAddr(prev.other);
            break;

    }

}
void PipeStg::MEMstage::execute() {
    // Set MEM/WB register
    next.op = prev.op;
    next.func = prev.func;
    next.write_back = prev.write_back;
    next.rd = prev.rd;
    next.alu_result = prev.alu_result;

    // if halt 0x3f
    if(next.op == 0x3f) {
        next.halt = true;
        return;
    } else {
        next.halt = false;
    }

    int value, target;
    unsigned int uint;
    int sum = prev.alu_result;
    if(prev.mem_rw) {
        switch(prev.op) {
            case OP_LW:
                next.mem_data = sim->raw_data[sum / WORD];
                break;
            case OP_LH:
            case OP_LHU:
                value = sim->raw_data[sum / WORD] << BYTE * (sum % WORD);
                if ((value & SIGN_BIT) && (prev.op == OP_LH))
                    next.mem_data = value >> 2 * BYTE;
                else {
                    uint = (unsigned int) value;
                    next.mem_data = uint >> 2 * BYTE;
                }
                break;
            case OP_LB:
            case OP_LBU:
                value = sim->raw_data[sum / WORD] << BYTE * (sum % WORD);
                if ((value & SIGN_BIT) && (prev.op == OP_LB))
                    next.mem_data = value >> 3 * BYTE;
                else {
                    uint = (unsigned int) value;
                    next.mem_data = uint >> 3 * BYTE;
                }
                break;
            case OP_SW:
                sim->raw_data[sum / WORD] = sim->registers[prev.alu_result];
                break;
            case OP_SH:
                value = sim->registers[prev.alu_result];
                target = sim->raw_data[sum / WORD];
                if (sum % WORD == 0)
                    target = (value & 0xffff) << 2 * BYTE | (target & 0xffff);
                else if (sum % WORD == 2)
                    target = (value & 0xffff) | (target & 0xffff0000);
                sim->raw_data[sum / WORD] = target;
                break;
            case OP_SB:
                value = sim->registers[prev.alu_result];
                target = sim->raw_data[sum / WORD];
                if (sum % WORD == 0)
                    target &= 0x00ffffff;
                else if (sum % WORD == 1)
                    target &= 0xff00ffff;
                else if (sum % WORD == 2)
                    target &= 0xffff00ff;
                else
                    target &= 0xffffff00;
                target |= (value & 0xff) << (3 - sum % WORD) * BYTE;
                sim->raw_data[sum / WORD] = target;
                break;
        }
    }
}

void PipeStg::WBstage::execute() {
    if(prev.op == 0x3f) {
        prev.halt = true;
        return;
    } else {
        prev.halt = false;
    }
    if(prev.write_back) {
        if(prev.op >= OP_LB && prev.op <= OP_LW)
            sim->registers[prev.rd] = prev.mem_data;
        else
            sim->registers[prev.rd] = prev.alu_result;
    }
}

void PipeStg::OneCycle() {
    wbs->prev_name = mems->prev_name;
    mems->prev_name = exs->prev_name;
    exs->prev_name = ids->prev_name;

    sim->halt = false;
    int sum1 = 0, sum2 = 0;
    if(!sim->checkError(&sum1, &sum2)) {

        forwardEX();    // detect forwarding EX/DM to EX

        if(!sim->brk)
            wbs->execute();
        mems->execute();
        exs->execute(sum1);
        ids->execute();
        ifs->execute();
        if(sim->flush)
            sim->nowPC = sum2;
        if(!sim->forwardEX && sim->forwardID)
            forwardID();    // detect forwarding EX/DM to ID


        if(ifs->next.halt && ids->next.halt && exs->next.halt && mems->next.halt && wbs->prev.halt)
            sim->halt = true;

        updateReg();
    } else {
        ids->execute();
        ifs->execute();
    }
}

void PipeStg::forwardEX() {
    sim->forwardEX = false;
    if(sim->ps.mems->prev.write_back && (sim->ps.mems->prev.rd != '\0') &&
       (sim->ps.mems->prev.rd == sim->ps.exs->prev.rt) && sim->ps.exs->prev.op != OP_HALT) {
        sim->ps.exs->prev.rt_val = sim->ps.mems->prev.alu_result;
        sim->forwardEX = true;
        sim->stall = false;
        std::stringstream ss;
        ss << "rt_$" << (int)sim->ps.exs->prev.rt;
        ss >> sim->for_ex;    }
    if(sim->ps.mems->prev.write_back && (sim->ps.mems->prev.rd != '\0') &&
       (sim->ps.mems->prev.rd == sim->ps.exs->prev.rs) && sim->ps.exs->prev.op != OP_HALT) {
        sim->ps.exs->prev.rs_val = sim->ps.mems->prev.alu_result;
        sim->forwardEX = true;
        sim->stall = false;
        std::stringstream ss;
        ss << "rs_$" << (int)sim->ps.exs->prev.rs;
        ss >> sim->for_ex;
    }
}

void PipeStg::forwardID() {
    sim->forwardID = false;
    if(sim->ps.mems->prev.write_back && ((int)sim->ps.mems->prev.rd != 0) &&
            (sim->ps.mems->prev.rd == sim->ps.ids->next.rt) && sim->ps.ids->prev.ori != 0xffffffff) {
        sim->ps.ids->next.rt_val = sim->ps.mems->prev.alu_result;
        sim->forwardID = true;
        sim->stall = false;
        std::stringstream ss;
        ss << "rt_$" << (int)sim->ps.ids->next.rt;
        ss >> sim->for_id;
    }
    if(sim->ps.mems->prev.write_back && ((int)sim->ps.mems->prev.rd != 0) &&
       (sim->ps.mems->prev.rd == sim->ps.ids->next.rs) && sim->ps.ids->prev.ori != 0xffffffff) {
        sim->ps.ids->next.rs_val = sim->ps.mems->prev.alu_result;
        sim->forwardID = true;
        sim->stall = false;
        std::stringstream ss;
        ss << "rs_$" << (int)sim->ps.ids->next.rs;
        ss >> sim->for_id;
    }
}

void PipeStg::updateReg() {
    if(sim->flush) {
        ids->prev = *new PipeReg::IFID();
    } else if(!sim->stall)
        ids->prev = ifs->next;
    exs->prev = ids->next;
    mems->prev = exs->next;
    wbs->prev = mems->next;
}
