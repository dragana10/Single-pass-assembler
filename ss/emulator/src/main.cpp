#include <iostream>
#include <string>
#include <fstream>

#include "../inc/emulator.hpp"

using namespace std;

int main(int argc, const char* argv[]){

for(int i = 0; i<argc ; i++){
    cout << "i=" << i << " " << argv[i] << endl;
}
    string inputFileName = "";
    if(argc!=2){
        cout << "Wrong parameters.";
        return 0;
    }
    string par = argv[0];
    if(par.compare("./emulator")!=0){
        cout << "First parameter has to be 'emulator'.";
        return 0;
    }
    par = argv[1];
    inputFileName = par;
    if(par.find_first_of('.')!=string::npos){
        par = par.substr( par.find_first_of('.') + 1);
        if(par.compare("hex")!=0){
            cout << "Second parameter has to be '.hex' type of file.";
            return 0;
        }
    }

    Emulator e = Emulator(inputFileName);
    e.emulate();


    return 0;
}