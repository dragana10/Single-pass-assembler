#ifndef _FORWARD_REFERENCE_TABLE_
#define _FORWARD_REFERENCE_TABLE_

#include "symbol.hpp"

#include <string>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;



class ForwardReferenceTable
{

public:

    static int global_id;
    static std::list<Symbol*> forward_reference_table_list_of_all_symbols;

    static void add_symbol_to_list(Symbol& symbol);
    static void write_table();
    static Symbol* get_symbol_by_name(std::string name);
    static int is_simbol_in_section(std::string name, std::string section);

    
    int get_id();
    std::string get_name();
    int get_value();
    int get_size();
    int get_type();
    std::string get_section();
    int is_global();
    int is_defined();
    int is_part_of_global_dir();
    int is_part_of_extern_dir();

    void set_id(int id);
    void set_name(std::string name);
    void set_value(int value);
    void set_size(int size);
    void set_type(int type);
    void set_section(std::string section);
    void set_local();
    void set_global();
    void set_as_defined();
    void set_as_part_of_extern_dir();

    friend ostream& operator<< (ostream& os, const Symbol& symbol);

private:

};


#endif