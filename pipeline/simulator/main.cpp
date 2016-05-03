#include <iostream>
#include <assert.h>
#include <iomanip>
#include "simulator.h"

using namespace std;

const char* iimage_path = (char*)"iimage.bin";
const char* dimage_path = (char*)"dimage.bin";
const char* error_path = (char*)"error_dump.rpt";
const char* snapshot_path = (char*)"snapshot.rpt";

int cycle = 0;
fstream snapshot, error;

Simulator* sim;
array<int,RegNum> registers;

void Initialize();
void loadIimage();
void loadDimage();
void writeReg();

int main() {

    sim = new Simulator();
    Initialize();

    while(cycle <= MAX_CYCLE) {
        sim->ps.OneCycle();
        writeReg();
        if(sim->halt) {
            //writeReg();
            break;
        }
    }

    cout << "Simulation succeed." << endl;
    return 0;
}


void Initialize() {

    loadIimage();
    loadDimage();
    error.open(error_path, ios::out);
    error.close();

    snapshot.open(snapshot_path, ios::out);
    snapshot.close();
    sim->nowPC = sim->registers[PCReg];
    sim->ps.OneCycle();
    registers = sim->registers;
    writeReg();
}

void loadIimage() {
    sim->raw_instr = new int[MAX_MEMORY/WORD];

    fstream iimage;
    iimage.open(iimage_path, ios::binary | ios::in);
    assert(iimage);

    int num;
    for(int i = 0; i < 2; i++) {
        unsigned char tmp[4];
        iimage.read((char*)tmp, WORD);
        num = (int)tmp[0]<<24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        if(i == 0) {
            sim->registers[PCReg] = num;
            sim->PC_init = num;
        }
    }

    for(int i = 0; i < num; i++) {
        unsigned char tmp[4];
        iimage.read((char*)tmp, WORD);
        int instr = (int)tmp[0] << 24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        sim->raw_instr[i] = instr;
    }

    iimage.close();
}
void loadDimage() {
    sim->raw_data = new int[MAX_MEMORY/WORD];

    fstream dimage;
    dimage.open(dimage_path, ios::binary | ios::in);
    assert(dimage);

    int num;
    for(int i = 0; i < 2; i++) {
        unsigned char tmp[4];
        dimage.read((char*)tmp, WORD);
        num = (int)tmp[0]<<24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        if(i == 0)
            sim->registers[StackReg] = num;
    }

    for(int i = 0; i < num; i++) {
        unsigned char tmp[4];
        dimage.read((char*)tmp, WORD);
        int data = (int)tmp[0] << 24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        sim->raw_data[i] = data;
    }

    dimage.close();
}

void writeReg() {
    snapshot.open(snapshot_path, ios::out | ios::app);

    snapshot << "cycle " << dec << cycle << endl;
    for(int i = 0; i < PCReg; i++) {
        snapshot << "$" << setw(2) << setfill('0') << dec << i << ": 0x" << setw(8) << setfill('0') << hex << uppercase << registers[i] << endl;
    }
    snapshot << "PC: 0x" << setw(8) << setfill('0') << hex << uppercase << sim->registers[PCReg] << endl;

    if(sim->flush) {
        snapshot << "IF: 0x" << sim->ps.ifs->prev_name << " to_be_flushed" << endl;
        snapshot << "ID: " << sim->ps.ids->prev_name << endl;
    } else if(sim->stall) {
        snapshot << "IF: 0x" << sim->ps.ifs->prev_name << " to_be_stalled" << endl;
        snapshot << "ID: " << sim->ps.ids->prev_name << " to_be_stalled" << endl;
        sim->ps.ids->prev_name = "NOP";
    } else {
        snapshot << "IF: 0x" << sim->ps.ifs->prev_name << endl;
        if(sim->forwardID) {
            snapshot << "ID: " << sim->ps.ids->prev_name << " fwd_EX-DM_" << sim->for_id << endl;
        } else {
            snapshot << "ID: " << sim->ps.ids->prev_name << endl;
        }
    }
    if(sim->forwardEX) {
        snapshot << "EX: " << sim->ps.exs->prev_name << " fwd_EX-DM_" << sim->for_ex << endl;
    } else {
        snapshot << "EX: " << sim->ps.exs->prev_name << endl;
    }
    snapshot << "DM: " << sim->ps.mems->prev_name << endl;
    snapshot << "WB: " << sim->ps.wbs->prev_name <<endl;

    snapshot << "\n\n";
    snapshot.close();

    cycle++;
    registers = sim->registers;
    if(sim->stall) {
        sim->nowPC -= WORD;
        sim->stall = false;
    }
    if(sim->flush) {
        sim->flush = false;
    }
    if(sim->forwardID || sim->forwardEX) {
        sim->forwardID = false;
        sim->forwardEX = false;
    }
}

