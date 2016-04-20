//
// Created by selly on 2016/4/7.
//

#ifndef REGISTER_H
#define REGISTER_H

#define StackReg		30
#define ReturnAddrReg	31
#define PCReg			32
#define NextPCReg		33
#define PrevPCReg		34
#define RegNum			35

class Register {
public:

	Register();



	int registers[RegNum];
	int init_PC;
};



#endif //REGISTER_H

