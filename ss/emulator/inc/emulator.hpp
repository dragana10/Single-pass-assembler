#ifndef EMULATOR_H
#define EMULATOR_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <array>
#include <vector>

using namespace std;

#define MAX_MEMORY_SIZE 65536*2
#define MAX_REGISTERS_SIZE 9
#define SP_INITIAL_VALUE 0xfefe
#define REGISTER_PSW 8
#define REGISTER_PC 7
#define REGISTER_SP 6
#define INC_PC 1
#define INC_SP 1
#define PSW_SET_Z 0x0001
#define PSW_CLEAR_Z 0xfffe
#define PSW_SET_O 0x0002
#define PSW_CLEAR_O 0xfffd
#define PSW_SET_C 0x0004
#define PSW_CLEAR_C 0xfffb
#define PSW_SET_N 0x0008
#define PSW_CLEAR_N 0x0007


class Emulator{
public:
    Emulator(string inputFileName);
    void emulate();
    void fillMemory();
    void printMemory();

    void executeInstruction();
    short readOneByte(unsigned short ind);
    short readTwoBytes(unsigned short ind);
    short readTwoBytesBigIndexes(int ind);
    short readOneByteWithoutPcUpdating(unsigned short ind);
    void updateTheSelecterRegister(unsigned short regNum, unsigned short updateMode);
    void insertIntoMemory(unsigned short ind, string valueToInsert, int numOfBytes);
    void processJmpCallLdrInstruction(int updateMode, int addrMode, int regSrc, unsigned short regD);
    string getHexStringFromInt(int value);
    void registersValues();

private:
    ifstream* inputFile;
    ofstream out1;
    ofstream printMem;
    ofstream outEr;
    vector<unsigned char> memory; //size of char is 1
    vector<unsigned short> registers; //size of short is 2
    int isEnd;
};






#endif