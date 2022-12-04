#include <iostream>
#include <regex>
#include <list>
#include "../inc/linker.hpp"
using namespace std;

//g++ -o linker ./src/*.cpp
//./linker -hex -place=myData@0x3e -place=isr@0x0 -o ime1.hex tests/aisr_reset.o tests/aisr_terminal.o tests/aisr_timer.o tests/aivt.o

int main(int argc, char* argv[]){
    if(argc < 5){
        cout << "Wrong number of parameters" << endl;
        return 0;
    }

    string type = argv[1];
    if(type.compare("-hex")!=0 && type.compare("-relocateable")!=0){
        cout << "Second parameter must be '-hex' or '-relocateable" << endl;
        return 0;
    }

    type = argv[2];
    list<string> listOfSections;
    list<string> listOfAddresses;
    list<string> listOfInputFiles;
    string fileOut = "";
    int indHex = -1;
    if(type.find_first_of("place") != string::npos){
        cout << "Linker with -place option" << endl;
        for(int i = 2; i <argc ; i++){
            string tmp = argv[i];
            if(tmp.find_first_of("=") != string::npos){
                int ind0 = tmp.find_first_of("=");
                int ind1 = tmp.find_first_of("@");
                // cout << ind0 << " " << ind1 << endl;
                listOfSections.push_back(tmp.substr(ind0+1,ind1-ind0-1));
                listOfAddresses.push_back(tmp.substr(ind1+1));
            }
            if(tmp.compare("-o")==0){
                indHex = i+1;
            }
            if(tmp.find_first_of("/")!=string::npos){
                listOfInputFiles.push_back(tmp);
            }
        }
        fileOut = argv[indHex];
        int j = 0;
        for(auto i = listOfSections.begin(); i !=listOfSections.end(); i++, j++){
            string tmp = *i;
            cout << "listOfSections[" << j << "]="<< tmp << endl;
        }
        j = 0;
        for(auto i = listOfAddresses.begin(); i !=listOfAddresses.end(); i++, j++){
            string tmp = *i;
            cout << "listOfAddresses[" << j << "]="<< tmp << endl;
        }
        j = 0;
        for(auto i = listOfInputFiles.begin(); i !=listOfInputFiles.end(); i++, j++){
            string tmp = *i;
            cout << "listOfInputFiles[" << j << "]="<< tmp << endl;
        }
        Linker l = Linker(listOfSections,listOfAddresses,listOfInputFiles,fileOut);
        l.linker();
    }
    else{
        cout << "Linker without -place option" << endl;
        for(int i = 2; i <argc ; i++){
            string tmp = argv[i];
            if(tmp.compare("-o")==0){
                indHex = i+1;
            }
            if(tmp.find_first_of("/")!=string::npos){
                listOfInputFiles.push_back(tmp);
            }
        }
        fileOut = argv[indHex];
        int j = 0;
        for(auto i = listOfInputFiles.begin(); i !=listOfInputFiles.end(); i++, j++){
            string tmp = *i;
            cout << "listOfInputFiles[" << j << "]="<< tmp << endl;
        }
        Linker l = Linker(listOfSections,listOfAddresses,listOfInputFiles,fileOut);
        l.linker();
    }
    return 0;
}