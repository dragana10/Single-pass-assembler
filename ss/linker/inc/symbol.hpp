#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <iostream>
#include <iterator>
#include <list>

#include "section.hpp"

using namespace std;



class Symbol
{

public:

    static std::list<Symbol*> global_sym_list;

    static void add_symbol_to_list(Symbol& symbol);
    static void write_table();
    static Symbol* get_symbol_by_name(std::string name);

    Symbol(int id, std::string name, int value, std::string section, int binding, string input);
    
    int get_id();
    std::string get_name();
    int get_value();
    int get_size();
    std::string get_section();
    Section* get_section_object();
    int is_global();
    string getInputFile();
    Section* getSectionInOutputFile();

    void set_id(int id);
    void set_name(std::string name);
    void set_value(int value);
    void set_size(int size);
    void set_section(std::string section);
    void set_section_object(Section* section);
    void set_local();
    void set_global();
    void setSectionInOutputFile(Section* s);

    friend ostream& operator<< (ostream& os, const Symbol& symbol);


private:

    int id;
    std::string name;
    int value; //section offset
    int size; //object size in bytes
    std::string section;
    Section* section_object;
    Section* sectionInOutputFile;
    int binding; // 0-local ; 1-global
    string inputFile;


};


#endif