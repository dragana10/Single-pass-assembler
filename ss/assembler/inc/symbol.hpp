#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <iostream>
#include <iterator>
#include <list>

#include "section.hpp"

using namespace std;

#define SECTION_ABS 0
#define SECTION_UNDEF 1

#define SimType_NOT_DEF 0
#define SimType_DATA 1
#define SimType_SECTION 2
#define SimType_ABS 1
#define SimType_FUNC 3
#define SimType_SRC_FILE_NAME 4

struct ST_forwardrefs {
    int loc_cnt;          // adresa u programu za izmenu  
    int size;
    Section* section;
    RelocationRecordType type;
    ST_forwardrefs *next;// pokazivač na sledeći zapis

    ST_forwardrefs(int l, int s, Section* sec, RelocationRecordType t){
        loc_cnt = l;
        size = s;
        section=sec;
        type = t;
        next = nullptr;
    }

};


class Symbol
{

public:

    static int global_id;
    static std::list<Symbol*> global_sym_list;

    static void add_symbol_to_list(Symbol& symbol);
    static void write_table();
    static Symbol* get_symbol_by_name(std::string name);
    static int is_simbol_in_section(std::string name, std::string section);

    Symbol(std::string name, int value, int size, int type, std::string section, int binding, int is_defined_flag, int is_part_of_global_dir);
    Symbol();
    
    int get_id();
    std::string get_name();
    int get_value();
    int get_size();
    int get_type();
    std::string get_section();
    Section* get_section_object();
    ST_forwardrefs* get_flink();
    int is_global();
    int is_defined();
    int is_part_of_global_dir();
    int is_part_of_extern_dir();
    int is_part_of_word_dir();

    void set_id(int id);
    void set_name(std::string name);
    void set_value(int value);
    void set_size(int size);
    void set_type(int type);
    void set_section(std::string section);
    void set_section_object(Section* section);
    void set_local();
    void set_global();
    void set_as_defined();
    void set_as_part_of_extern_dir();
    void set_as_part_of_word_dir();
    void insert_flink(int loc_cnt, int size, Section* section, RelocationRecordType type);
    void add_relocation_record(RelocationRecord& rel);

    friend ostream& operator<< (ostream& os, const Symbol& symbol);

    std::list<RelocationRecord*> list_of_my_relocation_records;

private:

    int id;
    std::string name;
    int value; //section offset
    int size; //object size in bytes
    int type;
    std::string section;
    Section* section_object;
    int binding; // 0-local ; 1-global
    int is_defined_flag;
    int part_of_global_dir;
    int part_of_extern_dir;
    int part_of_word_dir;
    ST_forwardrefs* flink;// početak liste obraćanja unapred


};


#endif