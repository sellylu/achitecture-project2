//
// Created by selly on 2016/4/26.
//

#ifndef PIPEREG_H
#define PIPEREG_H

#include "instruction.h"

class PipeReg {
public:

    class latch {
    public:
        int op = 0;
        int func = 0;
        bool halt = false;
    };

    class IFID{
    public:
        int ori = 0;
        int PC = 0;
        bool halt = false;
    };
    class IDEX : public latch {
    public:
        bool write_back = false, mem_rw = false;
        int PC = 0;
        int  rs_val = 0, rt_val = 0;
        char rs = 0, rt = 0, rd = 0;
        int other = 0;
        void copyInstr(Instruction* i) {
            this->rs = i->rs;
            this->rt = i->rt;
            this->rd = i->rd;
            this->other = i->other;
            this->op = i->op;
            this->func = i->func;
        }
    };
    class EXMEM : public latch {
    public:
        bool write_back = false, mem_rw = false;
        int alu_result;
        char rd;
    };
    class MEMWB : public latch {
    public:
        bool write_back = false;
        int mem_data, alu_result;
        char rd;
    };

};


#endif //PIPEREG_H

