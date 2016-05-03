//
// Created by selly on 2016/4/26.
//

#ifndef PIPESTG_H
#define PIPESTG_H

#include "pipereg.h"
#include <string>

#define MAX_CYCLE   500000
#define MAX_MEMORY  0x400
#define MIN_MEMORY	0x0

#define WORD        4
#define BYTE        8
#define SIGN_BIT    0x80000000
#define IndexToAddr(x) ((x) << 2)

#define StackReg        29
#define ReturnAddrReg   31
#define PCReg           32
#define RegNum          33

class Simulator;

class PipeStg {
public:
    PipeStg(Simulator*);

    class IFstage {
    public:
        IFstage(Simulator* s) {
            sim = s;
        }
        void execute();

        std::string prev_name = "00000000";
        Simulator *sim;
        PipeReg::IFID next;

        void doNOP() { next.ori = 0; }
    };
    class IDstage {
    public:
        IDstage(Simulator* s) {
            sim = s;
        }
        void execute();

        std::string prev_name = "NOP";
        Simulator *sim;
        PipeReg::IFID prev;
        PipeReg::IDEX next;

        void doNOP() {
            next = *new PipeReg::IDEX();
            next.PC -= WORD;
        }
    };
    class EXstage{
    public:
        EXstage(Simulator* s) {
            sim = s;
        }
        void execute(int);

        std::string prev_name = "NOP";
        Simulator *sim;
        PipeReg::IDEX prev;
        PipeReg::EXMEM next;


        void doNOP() {
            next.op = R_FORMAT;
            next.func = FUNC_SLL;
        }

    };
    class MEMstage{
    public:
        MEMstage(Simulator* s) {
            sim = s;
        }
        void execute();

        std::string prev_name = "NOP";
        Simulator *sim;
        PipeReg::EXMEM prev;
        PipeReg::MEMWB next;


        void doNOP() {
            next.op = R_FORMAT;
            next.func = FUNC_SLL;
        }

    };
    class WBstage {
    public:
        WBstage(Simulator* s) {
            sim = s;
        }
        void execute();

        std::string prev_name = "NOP";
        Simulator *sim;
        PipeReg::MEMWB prev;

        void doNOP() {
            prev.op = R_FORMAT;
            prev.func = FUNC_SLL;
        }
    };

    IFstage* ifs;
    IDstage* ids;
    EXstage* exs;
    MEMstage* mems;
    WBstage* wbs;
    Simulator* sim;

    void OneCycle();
    void updateReg();
    void forwardEX();
    void forwardID();

};

static std::string getInstrName(int op, int func) {

    switch (op) {
        case R_FORMAT:

            switch (func) {
                case FUNC_ADD:
                    return "ADD";
                case FUNC_ADDU:
                    return "ADDU";
                case FUNC_SUB:
                    return "SUB";
                case FUNC_AND:
                    return "AND";
                case FUNC_OR:
                    return "OR";
                case FUNC_XOR:
                    return "XOR";
                case FUNC_NOR:
                    return "NOR";
                case FUNC_NAND:
                    return "NAND";
                case FUNC_SLT:
                    return "SLT";
                case FUNC_SLL:
                    return "SLL";
                case FUNC_SRL:
                    return "SRL";
                case FUNC_SRA:
                    return "SRA";
                case FUNC_JR:
                    return "JR";
            }
        case OP_ADDI:
            return "ADDI";
        case OP_ADDIU:
            return "ADDIU";
        case OP_LW:
            return "LW";
        case OP_LH:
            return "LH";
        case OP_LHU:
            return "LHU";
        case OP_LB:
            return "LB";
        case OP_LBU:
            return "LBU";
        case OP_SW:
            return "SW";
        case OP_SH:
            return "SH";
        case OP_SB:
            return "SB";
        case OP_LUI:
            return "LUI";
        case OP_ANDI:
            return "ANDI";
        case OP_ORI:
            return "ORI";
        case OP_NORI:
            return "NORI";
        case OP_SLTI:
            return "SLTI";
        case OP_BEQ:
            return "BEQ";
        case OP_BNE:
            return "BNE";
        case OP_BGTZ:
            return "BGTZ";
        case OP_J:
            return "J";
        case OP_JAL:
            return "JAL";
        case OP_HALT:
            return "HALT";
    }
    return NULL;
}
#endif //PIPESTG_H


