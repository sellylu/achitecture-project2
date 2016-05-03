//
// Created by selly on 2016/4/26.
//

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "pipestg.h"
#include <fstream>
#include <array>

class Simulator {
public:
    Simulator():ps(this) {
    }
    Simulator(int* i, int* d):ps(this) {
        this->raw_instr = i;
        this->raw_data = d;
    }

    PipeStg ps;
    int *raw_instr;
    int PC_init = 0;
    int *raw_data;
    std::array<int,RegNum> registers;
    int nowPC = 0;

    bool halt = false;
	bool brk = false;
    bool stall = false;
    bool forwardID = false;
    std::string for_id = "";
    bool forwardEX = false;
    std::string for_ex = "";
    bool flush = false;

    bool checkError(int*, int*);
    void checkNumOverflow(int, int, int*);
    bool checkAddrOverflow(int, int);
    bool checkDataMisaligned(int, int);
    void writeError(int);

};

enum ERROR { WriteRegZero , NumOverflow, AddrOverflow, DataMisaligned };



extern const char* error_path;
extern std::fstream snapshot, error;
extern int cycle;


#endif //SIMULATOR_H

