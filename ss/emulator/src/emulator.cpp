#include<iostream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include<fstream>
#include<string>
#include<iterator>
#include<array>
#include<vector>
#include <bitset>

#include "../inc/emulator.hpp"

using namespace std;


Emulator::Emulator(string inputFileName){
    this->inputFile = new std::ifstream(inputFileName,std::ifstream::in);
    this->memory = vector<unsigned char>(MAX_MEMORY_SIZE,'0');
    this->registers = vector<unsigned short>(MAX_REGISTERS_SIZE,0);
    unsigned short sp = SP_INITIAL_VALUE;
    this->registers[REGISTER_SP] = sp;
    this->isEnd = 0;
    this->out1 = std::ofstream("myHelpFileEmulator.txt");
    this->printMem = std::ofstream("memoryPrinting.txt");
    this->outEr = std::ofstream("errorsEmulator.txt");
}

void Emulator::emulate(){
    if(this->inputFile->is_open()==false){
        this->outEr << "ERROR - emulation file can't open." << endl;
        return;
    }
    fillMemory();
    //printMemory();
    int cnt = 0;
    while(this->inputFile->eof() && this->isEnd==0){
        executeInstruction();
        cout << "cnt: " << cnt++ << endl;
        this->out1 << "cnt: " << cnt << endl;
        registersValues();
    }
    int width = 10;
    char sep = ' ';
    string str = "";

    cout << "--------------------------------------------" << endl;
    cout << "Emulated processor executed halt instruction  ";
    cout << "psw=" << std::bitset<16>(registers[8]) << endl;
    for(int i=0;i<8;i++){
        cout<< "r" << i << "=0x" << hex << registers[i];
        if(i!=3) cout << std::setw(width) << std::setfill(sep);
        if(i==3) cout << endl;
    }
    cout << endl;
    
}

void Emulator::registersValues(){
    int width = 10;
    char sep = ' ';
    cout << "psw=" << std::bitset<16>(registers[8]) << endl;
    for(int i=0;i<8;i++){
        cout<< "r" << i << "=" << hex << registers[i] << " ";
        if(i==3) cout << endl;
    }
    cout << endl;
    cout << "------------------------------------------------";
    cout << endl;
    cout << endl;
}

void Emulator::fillMemory(){
    string line = "", firstChar = "", tmp="";
    int ind = -1, iter = 0;
    string toInsert = "";
    getline(*inputFile,line);
    
    //this->out1 << "line is: #" << line << "#" << endl;
    while(this->inputFile->eof()==false){
        ind = line.find_first_of(':');
        if(ind==string::npos) break;
        ind+=2;
        line = line.substr(ind);
        int n = line.size();
        //this->out1 << "line is: #" << line << "#" << endl;
        for(int i = 0; i<n;i++){
            firstChar = line[0];
            //this->out1 << "firstChar is: #" << firstChar << "#" << endl;
            if(firstChar.compare(" ")==0) {
                //this->out1 << "continue..." << endl;
                line = line.substr(1);
                //i++;
                continue;
            };
            toInsert.push_back(line[0]);
            //toInsert.push_back(line[1]);
            //this->out1 << "toInsert is: #" << toInsert << "#" << endl;
            //line = line.substr(1);
            //this->out1 << "line is: #" << line << "#" << endl;
            unsigned char add = (unsigned char)stoi(toInsert,nullptr,16);
            toInsert = "";
            memory[iter++]=line[0];
            //i+=2;
            //cout << add << " ";
            line = line.substr(1);
            //this->out1 << "line is: #" << line << "#" << endl;

            //this->out1 << "inserted " << toInsert << " at position " << iter-1 << endl;
        }

        getline(*inputFile,line);
    }

    registers[REGISTER_PC] = readTwoBytes(0);
    //registers[REGISTER_PC] *= 2;
}

void Emulator::printMemory(){
    // for(int i=0;i<memory.size();i++){
    //     this->printMem << memory[i];
    //     if(i%2) this->printMem << " ";
    //     if(i%16==15) this->printMem << endl;
    // }
    int i = 0;
    for(;i<memory.size();){
        if(i%16 == 0){
            if(i != 0)this->printMem<< "\n";
            this->printMem<< right << setw(4) << setfill('0') << std::hex << i/2 << ": ";
        }
        this->printMem << std::hex << memory[i];
        if(i%2) this->printMem << " ";
        i++;
    }
    this->printMem<< endl;
   this->printMem << "\n";
}

void Emulator::executeInstruction(){
    this->out1 << "registers[REGISTER_PC] " << registers[REGISTER_PC] << endl;
    unsigned short currByte = readOneByte(registers[REGISTER_PC]);
    registers[REGISTER_PC] += INC_PC;
    this->out1 << "NEW INSTRUCTION : One byte from address " << registers[REGISTER_PC]-1 << " read, val: " << hex << currByte << endl;

    switch (currByte)
    {
    case 0x00: //HALT
    {
        this->out1 << "INSTRUCTION halt at address " << (registers[REGISTER_PC]-2)/2 << endl;
        this->isEnd = 1;
        break;
    }
    case 0x10:{ //int PROVERITI ????
        //push pc
        //push psw
        //pc<=mem[(reg[DDDD]%8)*2]
        //this->printMem << "BEFORE INT " << endl;
        //printMemory();
        cout << "int AT START-> pc: " << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        this->out1<< "INSTRUCTION int(push pc,push psw,pc<=mem[(reg[DDDD]%8)*2]), registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        short numD = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        this->out1 << "After instruction int read, registers[REGISTER_PC: " << registers[REGISTER_PC]  << endl;
        int num = (numD>>4) & 0xf;
        this->out1 << "dddd1111 read: " << hex << numD << ", registers[REGISTER_PC]: hex:" << registers[REGISTER_PC] << ", dec:" << dec << registers[REGISTER_PC] << endl;
        if(num < 0 || num >8 || (numD & 0x0f) != 0xf){
            this->outEr << "Format of int instruction is not correct." << endl;
            return;
        }
        this->out1 << "numD(real): " << hex << num << endl;


        //push pc
        this->out1 << "push pc... " << endl;
        string valToAdd = "";
        registers[REGISTER_SP] -= 2; //sp now points to free location
        unsigned short pc = registers[REGISTER_PC];
        valToAdd = getHexStringFromInt(pc);
        this->out1 << "we adding " << hex << valToAdd << " at address(stack): " << registers[REGISTER_SP] << endl;
        insertIntoMemory(registers[REGISTER_SP],valToAdd,2);

        //push psw
        this->out1 << "push psw... " << endl;
        valToAdd = "";
        registers[REGISTER_SP] -= 2; //sp now points to free location
        unsigned short psw = registers[REGISTER_PSW];
        valToAdd = getHexStringFromInt(psw);
        this->out1 << "we adding " << valToAdd << " at address(stack): " << registers[REGISTER_SP] << endl;
        insertIntoMemory(registers[REGISTER_SP],valToAdd,2);

        //PROVERI LITTLE ENDIAN???
        //pc<=mem[(reg[DDDD]%8)*2]
        unsigned short indToRead = (registers[num] % 8) * 2;
        this->out1 << "pc<=mem[(reg[DDDD]%8)*2], reg[dddd]: " << registers[num] << " (registers[num] % 8) * 2: " << indToRead << endl;
        unsigned short newOne = readTwoBytes(indToRead);
        registers[REGISTER_PC] = newOne;
        this->out1<< "AFTER INSTRUCTION int: pc=" << registers[REGISTER_PC] << endl;

        cout << "int AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        //this->printMem << "AFTER INT " << endl;
        //printMemory();
        break;
    }
    case 0x20:{ //iret
        //pop psw
        //pop pc
        //this->printMem << "BEFORE IRET " << endl;
        //printMemory();
        cout << "iret AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        cout << "memory[sp+1][sp][sp+3][sp+2]=" << memory[registers[REGISTER_SP]+1] << memory[registers[REGISTER_SP]] << memory[registers[REGISTER_SP]+3] << memory[registers[REGISTER_SP]+2] << endl;
        
        this->out1<< "INSTRUCTION iret(pop psw,pop pc, registers[REGISTER_PC]: " << registers[REGISTER_PC] << ", registers[REGISTER_PSW]: " << registers[REGISTER_PSW] << ", registers[REGISTER_SP]: " << registers[REGISTER_SP] << endl;
        short sp = registers[REGISTER_SP];
        this->out1 << "sp=" << sp << endl;
        cout << "sp=" << sp << endl;

        unsigned short newOne = readTwoBytes(sp);
        registers[REGISTER_PSW] = newOne;
        registers[REGISTER_SP] += 2;
        sp = registers[REGISTER_SP];
        this->out1 << "sp=" << sp << endl;
        cout << "sp=" << sp << endl;
        //this->out1 << "memory[sp+1][sp][sp+3][sp+2]=" << memory[sp+1] << memory[sp] << memory[sp+3] << memory[sp+2] << endl;
        newOne = readTwoBytes(sp);
        registers[REGISTER_PC] = newOne;
        registers[REGISTER_SP] += 2;
        this->out1<< "AFTER INSTRUCTION iret: pc=" << registers[REGISTER_PC] << ", psw=" << registers[REGISTER_PSW] << endl;

        cout << "iret AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        //this->printMem << "AFTER IRET " << endl;
        //printMemory();
        
        break;

    }
    case 0x30:{ //call
        //0x 30 fS UA OO OO
        //push pc
        //pc<=operand
        cout << "call AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1<< "INSTRUCTION call(push pc,pc<=operand, registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int regS = regsDescr & 0x0f;
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        unsigned short newPC = (addrMode==0x1 || addrMode==0x2)? registers[REGISTER_PC] : registers[REGISTER_PC]+2;
        this->out1<< "RegsDesc: " << regsDescr << ", addrMode: " << addrModeB << ", regS:" << regS << ", updareMode: " << updateMode << ", addrMode: " << addrMode << endl;

        if(regS<0 || regS>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of call instruction is not correct." << endl;
            return;
        }

        //push pc
        this->out1 << "push pc... " << endl;
        string valToAdd = "";
        registers[REGISTER_SP] -= 2; //sp now points to free location
        valToAdd = getHexStringFromInt(newPC);
        this->out1 << "we adding " << valToAdd << " at address(stack): " << registers[REGISTER_SP] << endl;
        insertIntoMemory(registers[REGISTER_SP],valToAdd,2);

        processJmpCallLdrInstruction(updateMode,addrMode,regS,REGISTER_PC);

        cout << "call AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;

    }
    case 0x40:{ //ret
        //pop pc
        this->printMem << "BEFORE RET " << endl;
        printMemory();
        cout << "ret AT START-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        this->out1<< "INSTRUCTION ret(pop pc-read pc from memory at address " << registers[REGISTER_SP] << ")" << endl;
        registers[REGISTER_PC] = readTwoBytes(registers[REGISTER_SP]);
        registers[REGISTER_SP] += 2;
        this->out1<< "AFTER INSTRUCTION ret: pc=" << registers[REGISTER_PC] << ", sp=" << registers[REGISTER_SP] << endl;
        cout << "ret AT END-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        this->printMem << "AFTER RET " << endl;
        printMemory();
        break;
    }
    case 0x50:{ //jmp
        cout << "jmp AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1<< "INSTRUCTION jmp, registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int regS = regsDescr & 0x0f;
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1<< "RegsDesc: " << regsDescr << ", addrMode: " << addrModeB << endl;

        if(regS<0 || regS>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of call instruction is not correct." << endl;
            return;
        }

        processJmpCallLdrInstruction(updateMode,addrMode,regS,REGISTER_PC);

        cout << "jmp AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x51:{ //jeq

        cout << "jeq AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1<< "INSTRUCTION jeq, registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        this->out1<< "registers[REGISTER_PSW] & PSW_SET_Z == 1 ? " << (registers[REGISTER_PSW] & PSW_SET_Z) << endl;
        
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int regS = regsDescr & 0x0f;
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1<< "RegsDesc: " << regsDescr << ", addrMode: " << addrModeB << endl;

        if(regS<0 || regS>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of call instruction is not correct." << endl;
            return;
        }
        
        if(registers[REGISTER_PSW] & PSW_SET_Z == 1) processJmpCallLdrInstruction(updateMode,addrMode,regS,REGISTER_PC);
        else{
            if(addrMode!=2 && addrMode!=3) registers[REGISTER_PC] += INC_PC + INC_PC;
        }
        
        cout << "jeq AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x52:{ //jne

        cout << "jne AT START-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        this->out1<< "INSTRUCTION jne, registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        this->out1<< "registers[REGISTER_PSW] & PSW_SET_Z == 0 ? " << (registers[REGISTER_PSW] & PSW_SET_Z) << endl;
        
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int regS = regsDescr & 0x0f;
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1<< "RegsDesc: " << regsDescr << ", addrMode: " << addrModeB << endl;

        if(regS<0 || regS>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of call instruction is not correct." << endl;
            return;
        }
        
        if(registers[REGISTER_PSW] & PSW_SET_Z == 0) processJmpCallLdrInstruction(updateMode,addrMode,regS,REGISTER_PC);
        else{
            if(addrMode!=2 && addrMode!=3) registers[REGISTER_PC] += INC_PC + INC_PC;
        }
        
        cout << "jne AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x53:{ //jgt

        cout << "jgt AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1<< "INSTRUCTION jeq, registers[REGISTER_PC]: " << registers[REGISTER_PC] << endl;
        this->out1<< "registers[REGISTER_PSW] & PSW_SET_Z == 0 ? " << (registers[REGISTER_PSW] & PSW_SET_Z) << endl;
        this->out1<< "(registers[REGISTER_PSW] & PSW_SET_N)==(registers[REGISTER_PSW] & PSW_SET_O) ? " << ((registers[REGISTER_PSW] & PSW_SET_N)==(registers[REGISTER_PSW] & PSW_SET_O)) << endl;
        
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int regS = regsDescr & 0x0f;
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1<< "RegsDesc: " << regsDescr << ", addrMode: " << addrModeB << endl;

        if(regS<0 || regS>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of call instruction is not correct." << endl;
            return;
        }
        
        if((registers[REGISTER_PSW] & PSW_SET_Z == 0) && ((registers[REGISTER_PSW] & PSW_SET_N)==(registers[REGISTER_PSW] & PSW_SET_O))) processJmpCallLdrInstruction(updateMode,addrMode,regS,REGISTER_PC);
        else{
            if(addrMode!=2 && addrMode!=3) registers[REGISTER_PC] += INC_PC + INC_PC;
        }
        
        cout << "jgt AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x60:{ //xchg
        cout << "xchg AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1<< "INSTRUCTION xchg(tmp=reg[dddd],reg[dddd]=reg[ssss],reg[ssss]=tmp " << endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of xchg instruction is not correct." << endl;
            return;
        }
        this->out1 << "before: tmp=" << registers[regD] << ", reg[dddd]=" << registers[regD] << ", reg[ssss]=" << registers[regS] << endl;
        regsDescr = registers[regD];
        registers[regD] = registers[regS];
        registers[regS] = regsDescr;
        this->out1 << "before: tmp=" << regsDescr << ", reg[dddd]=" << registers[regD] << ", reg[ssss]=" << registers[regS] << endl;

        cout << "xchg AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x70:{ //add

        cout << "add AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        this->out1 << "INSTRUCTION add at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of add instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] + registers[regS];
        cout << "add AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;
        break;
    }
    case 0x71:{ //sub
            cout << "sub AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION sub at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of sub instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] - registers[regS];
        cout << "sub AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x72:{ //mul
        cout << "mull AT START-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION mul at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of mul instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] * registers[regS];
            cout << "mull AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x73:{ //div
        cout << "div AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION div at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of mul instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] / registers[regS];
            cout << "div AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x74:{ //cmp
        cout << "cmp AT START-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION cmp at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of cmp instruction is not correct." << endl;
            return;
        }
        regsDescr = registers[regD] - registers[regS];
        if(regsDescr==0){
            registers[REGISTER_PSW] &= PSW_CLEAR_C;
            registers[REGISTER_PSW] &= PSW_CLEAR_N;
            registers[REGISTER_PSW] &= PSW_CLEAR_O;
            registers[REGISTER_PSW] |= PSW_SET_Z;
        }
        else if(regsDescr<0){
            registers[REGISTER_PSW] &= PSW_CLEAR_Z;
            if(regsDescr<0xffff0000) registers[REGISTER_PSW] &= PSW_CLEAR_O;
            else registers[REGISTER_PSW] |= PSW_SET_O;
            registers[REGISTER_PSW] |= PSW_SET_C;
            registers[REGISTER_PSW] |= PSW_SET_N;
        }
        else{
            registers[REGISTER_PSW] &= PSW_CLEAR_C;
            registers[REGISTER_PSW] &= PSW_CLEAR_N;
            registers[REGISTER_PSW] &= PSW_CLEAR_O;
            registers[REGISTER_PSW] &= PSW_CLEAR_Z;
        }
            cout << "cmp AT END-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x80:{ //not
        cout << "not AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION not at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of not instruction is not correct." << endl;
            return;
        }
        registers[regD] = ~(registers[regD]);
            cout << "not AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x81:{ //and
        cout << "and AT START-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION and at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of and instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] & registers[regS];
            cout << "and AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x82:{ //or
        cout << "or AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION or at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of or instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] | registers[regS];
            cout << "or AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x83:{ //xor
        cout << "xor AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION xor at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of xor instruction is not correct." << endl;
            return;
        }
        registers[regD] = registers[regD] ^ registers[regS];
            cout << "xor AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x84:{ //test
        cout << "test AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION test at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of test instruction is not correct." << endl;
            return;
        }
        regsDescr = registers[regD] & registers[regS];
        if(regsDescr==0){
            registers[REGISTER_PSW] &= PSW_CLEAR_N;
            registers[REGISTER_PSW] |= PSW_SET_Z;
        }
        else if(regsDescr & 0x8000){
            registers[REGISTER_PSW] &= PSW_CLEAR_Z;
            registers[REGISTER_PSW] |= PSW_SET_N;
        }
        else{
            registers[REGISTER_PSW] &= PSW_CLEAR_N;
            registers[REGISTER_PSW] &= PSW_CLEAR_Z;
        }
            cout << "test AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x90:{ //shl
        cout << "shl AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION shl at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of shl instruction is not correct." << endl;
            return;
        }
        int cFlag = registers[regD] << (registers[regS]-1);
        if(cFlag & 0x8000) registers[REGISTER_PSW] |= PSW_SET_C;
        else registers[REGISTER_PSW] &= PSW_CLEAR_C;

        registers[regD] = registers[regD] << registers[regS];
        if(regD==REGISTER_PC) registers[regD] *= 1; 

        if(registers[regD]==0) registers[REGISTER_PSW] |= PSW_SET_Z;
        else registers[REGISTER_PSW] &= PSW_CLEAR_Z;
        if(registers[regD] & 0x8000) registers[REGISTER_PSW] |= PSW_SET_N;
        else registers[REGISTER_PSW] &= PSW_CLEAR_N;
            cout << "shl AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0x91:{ //shr
        cout << "shr AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION shr at address " << (registers[REGISTER_PC]-2)/2<< endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0) >> 4;
        unsigned short regS = regsDescr & 0x000f;
        if(regD<0 || regD>8 || regS<0 || regS>8){
            this->outEr << "Format of shr instruction is not correct." << endl;
            return;
        }
        int cFlag = registers[regD] >> (registers[regS]-1);
        if(cFlag & 0x0001) registers[REGISTER_PSW] |= PSW_SET_C;
        else registers[REGISTER_PSW] &= PSW_CLEAR_C;

        registers[regD] = registers[regD] >> registers[regS];
        if(regD==REGISTER_PC) registers[regD] *= 1; 
        
        if(registers[regD]==0) registers[REGISTER_PSW] |= PSW_SET_Z;
        else registers[REGISTER_PSW] &= PSW_CLEAR_Z;
        if(registers[regD] & 0x8000) registers[REGISTER_PSW] |= PSW_SET_N;
        else registers[REGISTER_PSW] &= PSW_CLEAR_N;
            cout << "shr AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        break;
    }
    case 0xA0:{ //ldr
        //reg[DDDD]<=operand
            cout << "ldr AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION ldr at address " << (registers[REGISTER_PC]-2)<< endl;
        this->out1 << "INSTRUCTION ldr/pop" << endl;
        unsigned short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        unsigned short regD = (regsDescr & 0x00f0)>>4;
        unsigned short regS = regsDescr & 0x000f;        
        unsigned short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1 << "RegsDescr=" << hex << regsDescr << ", regD=" << hex << regD << ", regS=" << hex << regS << endl;
        this->out1 << "AddrMode=" << hex << addrModeB << ", updateMode=" << hex << updateMode << ", addrMode=" << hex << addrMode << endl;
        this->out1 << "registers[REGISTER_PC]=" << registers[REGISTER_PC] << endl;
        // if(regS==6 && addrModeB==0x42){
        //     //pop
        //     this->out1 << "pop r... " << regD << "=" << hex << registers[regD] << endl;
        //     unsigned short newOne = readTwoBytes(memory[registers[REGISTER_SP]]);
        //     registers[regD] = newOne;
        //     this->out1 << "r" << regD << "=" << hex << registers[regD] << endl;
        //     registers[REGISTER_SP] += 2;
        //         cout << "ldr AT END-> pc: "  << hex << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        //     break;
        // }
        // else{
            if(regD<0 || regD>8 || updateMode>4 || addrMode>5){
            this->outEr << "Format of ldr instruction is not correct." << endl;
            return;
            }  

            processJmpCallLdrInstruction(updateMode,addrMode,regS,regD);
            this->out1 << "registers[regD]=" << registers[regD] << endl;
                cout << "ldr AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

            break;
        // }
        
    }
    case 0xB0:{ //str
        //operand<=reg[DDDD]
        cout << "str AT START-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

        this->out1 << "INSTRUCTION str at address " << (registers[REGISTER_PC]-2)<< endl;
        this->out1 << "INSTRUCTION str/push" << endl;
        short regsDescr = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        short regD = (regsDescr & 0x00f0)>>4;
        short regS = regsDescr & 0x000f;
        short addrModeB = readOneByte(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC;
        int updateMode = (addrModeB & 0xf0) >> 4;
        int addrMode = addrModeB & 0x0f;
        this->out1 << "RegsDescr=" << hex << regsDescr << ", regD=" << hex << regD << ", regS=" << hex << regS << endl;
        this->out1 << "AddrMode=" << hex << addrModeB << ", updateMode=" << hex << updateMode << ", addrMode=" << hex << addrMode << endl;
        this->out1 << "registers[REGISTER_PC]=" << registers[REGISTER_PC] << endl;
        if(regS==6 && addrModeB==0x12){
            //push
            //push pc
            this->out1 << "push r... " << regD << "=" << hex << registers[regD] << endl;
            string valToAdd = "";
            registers[REGISTER_SP] -= 2; //sp now points to free location
            unsigned short val = registers[regD];
            valToAdd = getHexStringFromInt(val);
            this->out1 << "we adding " << hex << valToAdd << " at address(stack): " << registers[REGISTER_SP] << endl;
            insertIntoMemory(registers[REGISTER_SP],valToAdd,2);
                cout << "str AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

            break;
        }
        else{
            if(regD<0 || regD>8 || regS<0 || regS>0xf || updateMode>4 || addrMode>5 || addrModeB==0 || addrModeB==5){
            this->outEr << "Format of str instruction is not correct." << endl;
            return;
            }

            short addressOfOperand = -1;
            switch (addrMode)
            {
                case 1:{ //regdir
                    // 3 B has read
                    registers[regS] = registers[regD];
                    cout << "str AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

                    break;
                }
                case 2:{ //regind
                    // 3 B has read
                    // if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regS,updateMode);
                    // short first = readOneByteWithoutPcUpdating(registers[regS]);
                    // short second = readOneByteWithoutPcUpdating(registers[regS]+2);
                    // addressOfOperand = ((second<<8) & 0xff00) | (first & 0x00ff);
                    // if(updateMode==3 || updateMode==4)updateTheSelecterRegister(regS,updateMode);
                    // string valToAdd = to_string(registers[regD]);
                    // insertOneByteIntoMemory(addressOfOperand,valToAdd);
                    // valToAdd = to_string(registers[regD]>>8);
                    // insertOneByteIntoMemory(addressOfOperand+2,valToAdd);
                    if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regS,updateMode);
                    this->out1 << "registres[regS]=" << registers[regS];
                    short operand = registers[regS];
                    if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regS,updateMode);
                    string valToAdd = getHexStringFromInt(registers[regD]);
                    insertIntoMemory(operand,valToAdd,2);
                    cout << "str AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

                    break;
                }
                case 3:{ //regind with diplacement
                    // 3 B has read
                    // with readOneByte 2 B more has read
                    // if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regSrc,updateMode);
                    // short firstReg = readOneByteWithoutPcUpdating(registers[regSrc]);
                    // short secondReg = readOneByteWithoutPcUpdating(registers[regSrc]+2);
                    // short first = readOneByte(registers[REGISTER_PC]);
                    // short second = readOneByte(registers[REGISTER_PC]);
                    // short addrOfOperand = (((firstReg<<8) & 0xff00) | (secondReg & 0x00ff)) + (((first<<8) & 0xff00) | (second & 0x00ff));
                    // if(updateMode==3 || updateMode==4)updateTheSelecterRegister(regSrc,updateMode);
                    // insertOneByteIntoMemory(addressOfOperand,registers[regD]);
                    short operandPayload = readTwoBytes(registers[REGISTER_PC]);
                    registers[REGISTER_PC] += INC_PC + INC_PC;
                    if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regS,updateMode);
                    short address = registers[regS] + operandPayload;
                    if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regS,updateMode);
                    short operand = readTwoBytes(address);
                    string valToAdd = getHexStringFromInt(registers[regD]);
                    insertIntoMemory(operand,valToAdd,2);
                    cout << "str AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

                    break;
                }
                case 4:{ //mem
                    // 3 B has read
                    // with readOneByte 2 B more has read
                    // short first = readOneByte(registers[REGISTER_PC]);
                    // short second = readOneByte(registers[REGISTER_PC]);
                    // //operand = ((second<<8) & 0xff00) | (first & 0x00ff); //maybe bigEndian
                    // short addrOfOperand = ((first<<8) & 0xff00) | (second & 0x00ff); //big endian
                    // operand = readOneByteWithoutPcUpdating(addrOfOperand);
                    // registers[regD] = operand;
                    short operand = readTwoBytes(registers[REGISTER_PC]);
                    registers[REGISTER_PC] += INC_PC + INC_PC;
                    string valToAdd = getHexStringFromInt(registers[regD]);
                    insertIntoMemory(operand,valToAdd,2);
                    cout << "str AT END-> pc: " << hex  << registers[7] << ", sp: " << registers[6] << ", psw: " << registers[8]<< endl;

                    break;
                }
                // case 5:{ //regdir with add
                //     // 3 B has read
                //     // with readOneByte 2 B more has read
                //     short firstReg = readOneByteWithoutPcUpdating(registers[regSrc]);
                //     short secondReg = readOneByteWithoutPcUpdating(registers[regSrc]+2);
                //     short first = readOneByte(registers[REGISTER_PC]);
                //     short second = readOneByte(registers[REGISTER_PC]);
                //     short operand = (((firstReg<<8) & 0xff00) | (secondReg & 0x00ff)) + (((first<<8) & 0xff00) | (second & 0x00ff));
                //     registers[regD] = operand;
                //     break;
                //}
                default:
                    break;
            }

            break;
        }
        
    }
    default:
        break;
    }
}

void Emulator::processJmpCallLdrInstruction(int updateMode, int addrMode, int regSrc, unsigned short regD){
    short operand = -1;
    switch (addrMode)
    {
    case 0:{ //immmediate
        // 3 B has read already
        // 2 B more
        // short first = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short second = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        //operand = ((second<<8) & 0xff00) | (first & 0x00ff); //maybe bigEndian
        //operand = ((first<<8) & 0xff00) | (second & 0x00ff); //big endian
        operand = readTwoBytes(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC + INC_PC;
        if(regD != REGISTER_PC) registers[regD] = operand;
        else registers[regD] = operand;
        break;
    }
    case 1:{ //regdir
        // 3 B has read
        operand = registers[regSrc];
        if(regD != REGISTER_PC) registers[regD] = operand;
        else registers[regD] = operand;
        break;
    }
    case 2:{ //regind
        // 3 B has read
        if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regSrc,updateMode);
        // short first = readOneByteWithoutPcUpdating(registers[regSrc]);
        // short second = readOneByteWithoutPcUpdating(registers[regSrc]+2);
        // operand = ((first<<8) & 0xff00) | (second & 0x00ff);
        if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regSrc,updateMode);
        bool cond = (registers[regSrc] < 0x7fff);
        this->out1 << "(registers[regSrc] < 0x7fff)?" << cond << endl;
        if(cond)operand = readTwoBytes(registers[regSrc]);
        else{
                this->out1 << "registers[regSrc]  > 0x7fff, registers[regSrc]:" << registers[regSrc]  << endl;
                string str = "";
                int i = (registers[regSrc]*2) & 0x0001ffff;
                this->out1 << "i=" << hex << i << endl;
                str.push_back(memory[i+2]);
                str.push_back(memory[i+3]);
                short high = (short)(stoi(str,nullptr,16));
                str = "";
                str.push_back(memory[i]);
                str.push_back(memory[i+1]);
                short low = (short)(stoi(str,nullptr,16));
                short toRet = ((high << 8) & 0xff00 ) | (low & 0x00ff);
                operand = toRet;
        }
        this->out1 << "operand="<< operand << endl;
        if(updateMode==3 || updateMode==4)updateTheSelecterRegister(regSrc,updateMode);
        if(regD != REGISTER_PC) registers[regD] = operand;
        else registers[regD] = operand;
        break;
    }
    case 3:{ //regind with diplacement
        // 3 B has read
        // 2 B more 
        if(updateMode==1 || updateMode==2)updateTheSelecterRegister(regSrc,updateMode);
        // short firstReg = readOneByteWithoutPcUpdating(registers[regSrc]);
        // short secondReg = readOneByteWithoutPcUpdating(registers[regSrc]+2);
        // short first = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short second = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short addrOfOperand = (((firstReg<<8) & 0xff00) | (secondReg & 0x00ff)) + (((first<<8) & 0xff00) | (second & 0x00ff));
        // operand = ( (readOneByteWithoutPcUpdating(addrOfOperand) << 8) & 0xff00) | (readOneByteWithoutPcUpdating(addrOfOperand+2) & 0x00ff);
        this->out1 << "registers[regS]=" << registers[regSrc] << endl;
        short fromReg = registers[regSrc];
        short fromInstr = readTwoBytes(registers[REGISTER_PC]);
        this->out1 << "payload=" << fromInstr << endl;
        registers[REGISTER_PC] += INC_PC + INC_PC;
        unsigned int addr = 0xffffffff & (fromReg + fromInstr);
        this->out1 << "fromReg=" << fromReg << ", fromInstr" << fromInstr << endl;
        this->out1 << "address=" << addr << endl;
        bool cond = (addr < 0x7fff);
        this->out1 << "(addr < 0x7fff)?" << cond << endl;
        // if(cond)operand = readTwoBytes(addr);
        // else{
        //         this->out1 << "addr > 0x7fff, addr:" << addr << endl;
        //         string str = "";
        //         int i = addr & 0x0000ffff;
        //         this->out1 << "i*2=" << hex << i*2 << endl;
        //         str.push_back(memory[i*2+2]);
        //         str.push_back(memory[i*2+3]);
        //         short high = (short)(stoi(str,nullptr,16));
        //         str = "";
        //         str.push_back(memory[i*2]);
        //         str.push_back(memory[i*2+1]);
        //         short low = (short)(stoi(str,nullptr,16));
        //         short toRet = ((high << 8) & 0xff00 ) | (low & 0x00ff);
        //         operand = toRet;
        // }
        operand = readTwoBytes(addr);
        this->out1 << "operand=" << operand << endl;
        if(updateMode==3 || updateMode==4)updateTheSelecterRegister(regSrc,updateMode);
        if(regD != REGISTER_PC) registers[regD] = operand;
        else registers[regD] = operand;
        break;
    }
    case 4:{ //mem
        // 3 B has read
        // 2 B more
        // short first = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short second = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        //operand = ((second<<8) & 0xff00) | (first & 0x00ff); //maybe bigEndian
        // short addrOfOperand = ((first<<8) & 0xff00) | (second & 0x00ff); //big endian
        // operand = ( (readOneByteWithoutPcUpdating(addrOfOperand) << 8) & 0xff00) | (readOneByteWithoutPcUpdating(addrOfOperand+2) & 0x00ff);
        short addrOfOperand = readTwoBytes(registers[REGISTER_PC]);
        bool cond = (addrOfOperand < 0x7fff);
        this->out1 << "(addrOfOperand < 0x7fff)?" << cond << endl;
        if(cond)operand = readTwoBytes(addrOfOperand);
        else{
                this->out1 << "addr > 0x7fff, addr:" << addrOfOperand << endl;
                string str = "";
                int i = (addrOfOperand*2) & 0x0001ffff;
                this->out1 << "i=" << hex << i << endl;
                str.push_back(memory[i+2]);
                str.push_back(memory[i+3]);
                short high = (short)(stoi(str,nullptr,16));
                str = "";
                str.push_back(memory[i]);
                str.push_back(memory[i+1]);
                short low = (short)(stoi(str,nullptr,16));
                short toRet = ((high << 8) & 0xff00 ) | (low & 0x00ff);
                operand = toRet;
        }
        this->out1 << "operand="<< operand << endl;
        registers[REGISTER_PC] += INC_PC + INC_PC;
        if(regD != REGISTER_PC) registers[regD] = operand;
        else registers[regD] = operand;
        break;
    }
    case 5:{ //regdir with add
        // 3 B has read
        // 2 B more
        // short firstReg = readOneByteWithoutPcUpdating(registers[regSrc]);
        // short secondReg = readOneByteWithoutPcUpdating(registers[regSrc]+2);
        // short first = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short second = readOneByte(registers[REGISTER_PC]);
        // registers[REGISTER_PC] += INC_PC;
        // short operand = (((firstReg<<8) & 0xff00) | (secondReg & 0x00ff)) + (((first<<8) & 0xff00) | (second & 0x00ff));
        short fromReg = registers[regSrc];
        short fromInstr = readTwoBytes(registers[REGISTER_PC]);
        registers[REGISTER_PC] += INC_PC + INC_PC;
        operand= fromReg + fromInstr;
        registers[regD] = operand;
        this->out1 << "fromReg=" << fromReg << ", fromInstr=" << fromInstr << ", operand=fromReg+fromInstr=" << operand << endl;
        break;
    }
    default:
        break;
    }
}

void Emulator::updateTheSelecterRegister(unsigned short regNum, unsigned short updateMode){
    switch (updateMode)
    {
    case 0:
        break;
    case 1:{
        registers[regNum] -=2;
        break;
    }
    case 2:{
        registers[regNum] +=2;
        break;
    }
    case 3:{
        registers[regNum] -=2;
        break;
    }
    case 4:{
        registers[regNum] +=2;
        break;
    }
    default:
        break;
    }
}

short Emulator::readOneByte(unsigned short ind){
    if(!(ind & 0x8000)){
        ind *=2;
        this->out1 << "In memory, at index " << ind << " (address:" << ind/2 << ") is: " << memory[ind] << memory[ind+1] << endl;
        string str = "";
        str.push_back(memory[ind]);
        str.push_back(memory[ind+1]);
        //short toRet = memory[0] & 0xff;
        short toRet = (short)(stoi(str,nullptr,16));
        this->out1 << "returned value(that has read, we read 1 byte)): " << hex << toRet << endl;
        //registers[REGISTER_PC] += INC_PC;
        //this->out1 << "(after read one byte)registers[REGISTER_PC] " << registers[REGISTER_PC] << endl;
        return toRet;
    }
    else{
        string str = "";
        int i = (ind*2) & 0x0001ffff;
        this->out1 << "i=" << hex << i << endl;
        this->out1 << "In memory, at index " << i << " (address:" << i/2 << ") is: " << memory[ind] << memory[ind+1] << memory[ind+2] << endl;
        str.push_back(memory[i]);
        str.push_back(memory[i+1]);
        short toRet = (short)(stoi(str,nullptr,16));
        return toRet;
    }
}

short Emulator::readTwoBytes(unsigned short ind){
    if(!(ind & 0x8000)){
        ind *=2;
        cout << "readTwoBytes" << endl;
        this->out1 << "In memory, at index " << ind << " (address:" << ind/2 << ") is: " << memory[ind] << memory[ind+1] << memory[ind+2] << memory[ind+3] << endl;
        string str = "";
        str.push_back(memory[ind+2]);
        str.push_back(memory[ind+3]);
        short high = (short)(stoi(str,nullptr,16));
        str = "";
        str.push_back(memory[ind]);
        str.push_back(memory[ind+1]);
        short low = (short)(stoi(str,nullptr,16));
        short toRet = ((high << 8) & 0xff00 ) | (low & 0x00ff);
        this->out1 << "returned value(that has read, we read 2 bytes): " << hex << toRet << endl;
        //registers[REGISTER_PC] += INC_PC;
        //this->out1 << "(after read one byte)registers[REGISTER_PC] " << registers[REGISTER_PC] << endl;
        return toRet;
    }
    else{
        string str = "";
        int i = (ind*2) & 0x0001ffff;
        this->out1 << "i=" << hex << i << endl;
        this->out1 << "In memory, at index " << i << " (address:" << i/2 << ") is: " << memory[i] << memory[i+1] << memory[i+2] << memory[i+3] << endl;
        str.push_back(memory[i+2]);
        str.push_back(memory[i+3]);
        short high = (short)(stoi(str,nullptr,16));
        str = "";
        str.push_back(memory[i]);
        str.push_back(memory[i+1]);
        short low = (short)(stoi(str,nullptr,16));
        short toRet = ((high << 8) & 0xff00 ) | (low & 0x00ff);
        this->out1 << "toRet=" << hex << toRet << endl;
        return toRet;
    }
}

short Emulator::readTwoBytesBigIndexes(int ind){
    return 1;
}


short Emulator::readOneByteWithoutPcUpdating(unsigned short ind){
    cout << ind << " " << memory[ind] << memory[ind+1] << endl;
    string str = "";
    str.push_back(memory[ind]);
    str.push_back(memory[ind+1]);
    short toRet = (short)stoi(str,nullptr,16);
    cout << hex << toRet << endl;
    return toRet;
}

void Emulator::insertIntoMemory(unsigned short ind, string valueToInsert, int numOfBytes){
    //little endian
    if(valueToInsert.compare("")==0){
        valueToInsert = "0";
    }
    this->out1 << "insertIntoMemory --> ind: " << ind << ", val: " << valueToInsert << ", numOfBytes: " << numOfBytes << ", val.size: " << valueToInsert.size() << endl;

    if(!(ind & 0x8000)){
        ind*=2;
        this->out1 << "Want to insert at ind " << ind << ", address " << ind/2 << ", val " << valueToInsert << endl;
        int size = valueToInsert.size();
        if(size==1 && numOfBytes==1){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = '0';
            this->memory[ind+1] = valueToInsert[0];
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << endl;
        }
        else if(size==1 && numOfBytes==2){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = '0';
            this->memory[ind+1] = valueToInsert[0];
            this->memory[ind+2] = '0';
            this->memory[ind+3] = '0';
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << memory[ind+2] << memory[ind+3] << endl;
        }
        else if(size==2 && numOfBytes==1){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = valueToInsert[0];
            this->memory[ind+1] = valueToInsert[1];
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << endl;
        }
        else if(size==2 && numOfBytes==2){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = valueToInsert[0];
            this->memory[ind+1] = valueToInsert[1];
            this->memory[ind+2] = '0';
            this->memory[ind+3] = '0';
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << memory[ind+2] << memory[ind+3] << endl;
        }
        else if(size==3){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = valueToInsert[1];
            this->memory[ind+1] = valueToInsert[2];
            this->memory[ind+2] = '0';
            this->memory[ind+3] = valueToInsert[0];
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << memory[ind+2] << memory[ind+3] << endl;
        }
        else if(size==4){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[ind] = valueToInsert[2];
            this->memory[ind+1] = valueToInsert[3];
            this->memory[ind+2] = valueToInsert[0];
            this->memory[ind+3] = valueToInsert[1];
            this->out1 << "inserted: " << memory[ind] << memory[ind+1] << memory[ind+2] << memory[ind+3] << endl;
        }
    }
    else{
        int i = (ind*2) & 0x0001ffff;
        this->out1 << "i=" << hex << i << endl;
        this->out1 << "Want to insert at ind " << i << ", address " << i/2 << endl;
        int size = valueToInsert.size();
        if(size==1 && numOfBytes==1){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = '0';
            this->memory[i+1] = valueToInsert[0];
            this->out1 << "inserted: " << memory[i] << memory[i+1] << endl;
        }
        else if(size==1 && numOfBytes==2){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = '0';
            this->memory[i+1] = valueToInsert[0];
            this->memory[i+2] = '0';
            this->memory[i+3] = '0';
            this->out1 << "inserted: " << memory[i] << memory[i+1] << memory[i+2] << memory[i+3] << endl;
        }
        else if(size==2 && numOfBytes==1){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = valueToInsert[0];
            this->memory[i+1] = valueToInsert[1];
            this->out1 << "inserted: " << memory[i] << memory[i+1] << endl;
        }
        else if(size==2 && numOfBytes==2){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = valueToInsert[0];
            this->memory[i+1] = valueToInsert[1];
            this->memory[i+2] = '0';
            this->memory[i+3] = '0';
            this->out1 << "inserted: " << memory[i] << memory[i+1] << memory[i+2] << memory[i+3] << endl;
        }
        else if(size==3){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = valueToInsert[1];
            this->memory[i+1] = valueToInsert[2];
            this->memory[i+2] = '0';
            this->memory[i+3] = valueToInsert[0];
            this->out1 << "inserted: " << memory[i] << memory[i+1] << memory[i+2] << memory[i+3] << endl;
        }
        else if(size==4){
            this->out1 << "Value to insert in memory is " << valueToInsert << "(size: " << valueToInsert.size() << ")" << endl;
            this->memory[i] = valueToInsert[2];
            this->memory[i+1] = valueToInsert[3];
            this->memory[i+2] = valueToInsert[0];
            this->memory[i+3] = valueToInsert[1];
            this->out1 << "inserted: " << memory[i] << memory[i+1] << memory[i+2] << memory[i+3] << endl;
        }
    }
}

string Emulator::getHexStringFromInt(int value){
    string hexdec_num="";
    int r;
    char hex[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    while(value>0)
    {
        r = value % 16;
        hexdec_num = hex[r] + hexdec_num;
        value = value/16;
    }
    return  hexdec_num;
}