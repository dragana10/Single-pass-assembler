#include "../inc/assembler.hpp"
// #include "../inc/relocationrecord.h"
// #include "../inc/section.h"
// #include "../inc/symbol.hpp"

#include<iostream>

// g++ -o assembler ./src/*.cpp
// ./assembler -o izlaz.o tests/interrupts.s
// ./assembler -o out.o tests/my_test1.s

int main(int argc, const char *argv[])
{
    if(argv[1][0] != '-' || argv[1][1] != 'o') cout << "pogresni argumenti" << endl;
    else if (argc != 4){
        cout << "pogresan broj argumenata" << endl;
    }
    else {

        cout << "argv[0] " << argv[0] << endl;
        cout << "argv[1] " <<  argv[1] << endl;
        cout << "argv[2] " <<  argv[2] << endl;
        cout << "argv[3] " <<  argv[3] << endl;

        Assembler assembler(argv[3],argv[2]);
        assembler.single_pass_assembler();
        // assembler.backpatching(argv[3]);

    }
    return 0;

}