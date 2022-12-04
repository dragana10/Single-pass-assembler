#ifndef LINKER_H
#define LINKER_H

#include<fstream>
#include<sstream>
#include<iomanip>
#include<iostream>
#include<list>
#include<string>
#include<regex>
#include<iterator>
#include "section.hpp"
#include"symbol.hpp"
#include "relocationrecord.hpp"
using namespace std;


class Linker{

public:
    static int globalSymbolIds;
    static int globalSectionIds;
    string currSectionName;
    int endOfFile;

    Linker(list<string> sections, list<string> addresses, list<string> files, string fileOut);

    int getSizeOfPreviousSections();
    Section* getSectionByNameAndInputFile(string name, string inputFileName);
    Section* getSectionByName(string name);

    void linker();
    void readInputFiles();
    // void readCode(string filename);
    void readSymbolTable();
    void readRelocationTable();
    void readSections();
    void readTables();
    void mapSections();
    // void readRelocationRecords(string filename);
    // void readSectionContent(string filename);
    // void readSymbol(string line);
    // void writeOutputFile(string filename);
    int getNumericalValueFromString(string line, int isHex);
    int isThisSectionNameAdded(string name);
    int isSymbolInList(std::list<Symbol*> whichList, string name);
    void setSizeOfPreviousSections(int size);
    void solveTheSymbols();
    void resolveSymbols();
    void filterSymbols();
    int checkMultipleDefinitions(Symbol* symbol);
    int checkIsDefined(Symbol* symbol);
    Symbol* getDefinedSymbol(string name);
    void resolveRelocationRecords();
    void generateContent();
    void createNewSectionInOutputFile(string nameOfSection, Section* sec);
    void updateTheStartingAddressesOfAlreadyMappedSections(int sizeOfNewOne, Section* newOneOldSections, Section* newOneNewSections);
    void appendSection(Section* secToExpend, Section* secToAdd);
    void addSectionObjectsToSymbolsAndRelocationRecords();
    void addSectionObjectFromOutputFileToSymbols();
    std::list<Symbol*> removeSymbolsWithSpecificName(string name);
    void addContentAtSpecificPlace(int n, long data, int where);
    Symbol* getSymbolByName(string name);
    void printData();

    regex regexForOneLineSymTable;
    regex regexForOneLineRelTable;
    regex regexSectionNameSize;
    regex regexSectionContent;

private:
    ofstream out1;
    string regexSymTableString;
    string regexRelTableString;
    string regexSectionNameSizeString;
    string regexSectionContentString;
    string firstLineSymbolTable;
    string firstLineRelocationRecordTable;
    string firstLineSections;
    string currentInputFileName;
    fstream currentInputFile;
    std::list<Section*> listSections;
    std::list<Section*> listSectionsInOutputFile;
    std::list<Symbol*> listSymbols;
    std::list<RelocationRecord*> listRelocationRecords;
    std::list<string> listInputFilesNames;
    int sizeOfPreviousSections;
    std::list<int> data;
    ofstream fileOut;
};

#endif