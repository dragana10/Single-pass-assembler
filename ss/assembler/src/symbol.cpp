#include "../inc/symbol.hpp"

#include <string>
#include<sstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;

int Symbol::global_id=0;
std::list<Symbol*> Symbol::global_sym_list;


Symbol::Symbol(std::string name, int value, int size, int type, std::string section, int binding, int is_defined_flag, int is_part_of_global_dir)
{
    this->id=global_id++;
    this->name=name;
    this->value=value;
    this->size=size;
    this->type=type;
    this->section=section;
    this->section_object = Section::get_section_by_name(section);
    this->binding=binding;
    this->is_defined_flag=is_defined_flag;
    this->part_of_global_dir = is_part_of_global_dir;
    this->part_of_extern_dir = 0;
    this->part_of_word_dir = 0;
    this->flink = nullptr;
    Symbol::add_symbol_to_list(*this);
    cout << "val, sec: " << value << ", " << section << endl;
}

void Symbol::add_symbol_to_list(Symbol& symbol)
{
    cout << "Dodajem simbol #" << symbol.get_name() << "#------------------------------------------------------------" << endl;
    Symbol::global_sym_list.push_back(&symbol);
}

void Symbol::add_relocation_record(RelocationRecord& rel){
    this->list_of_my_relocation_records.push_back(&rel);
}

void Symbol::write_table()
{
    for (auto i = Symbol::global_sym_list.begin(); i != Symbol::global_sym_list.end(); i++)
    {
        cout << *i;
    }
    
}

Symbol* Symbol::get_symbol_by_name(string name)
{
    for (auto i = Symbol::global_sym_list.begin(); i != Symbol::global_sym_list.end(); i++)
    {
        Symbol* s = *i;
        if((s->name.compare(name))==0)
        {
            return s;
        }
    }
    return nullptr;
}

int Symbol::is_simbol_in_section(string name, string section)
{
    for (auto i = Symbol::global_sym_list.begin(); i != Symbol::global_sym_list.end(); i++)
    {
        Symbol* sym = *i;
        if((sym->section.compare(section))==0)
        {
            return 1;
        };
    };
    return 0;
}

int Symbol::get_id()
{
    return this->id;
}

string Symbol::get_name()
{
    return this->name;
}

int Symbol::get_value()
{
    return this->value;
}

int Symbol::get_size()
{
    return this->size;
}

int Symbol::get_type()
{
    return this->type;
}

string Symbol::get_section()
{
    return this->section;
}

Section* Symbol::get_section_object()
{
    return this->section_object;
}

ST_forwardrefs* Symbol::get_flink(){
    return this->flink;
}

int Symbol::is_global()
{
    return this->binding;
}

int Symbol::is_defined()
{
    return this->is_defined_flag;
}

int Symbol::is_part_of_global_dir()
{
    return this->part_of_global_dir;
}

int Symbol::is_part_of_extern_dir()
{
    return this->part_of_extern_dir;
}

int Symbol::is_part_of_word_dir()
{
    return this->part_of_word_dir;
}

void Symbol::set_id(int id)
{
    this->id=id;
}

void Symbol::set_name(string name)
{
    this->name=name;
}

void Symbol::set_value(int value)
{
    this->value=value;
}

void Symbol::set_size(int size)
{
    this->size=size;
}

void Symbol::set_type(int type)
{
    this->type=type;
}

void Symbol::set_section(string section)
{
    this->section=section;
}

void Symbol::set_section_object(Section* section)
{
    this->section_object=section;
}
    
void Symbol::set_local()
{
    this->binding=0;
}
    
void Symbol::set_global()
{
    this->binding=1;
}

void Symbol::set_as_defined()
{
    this->is_defined_flag=1;
}

void Symbol::set_as_part_of_extern_dir()
{
    this->part_of_extern_dir=1;
}

void Symbol::set_as_part_of_word_dir()
{
    this->part_of_word_dir=1;
}

void Symbol::insert_flink(int loc_cnt, int size, Section* section, RelocationRecordType type){
    if(flink==nullptr)
    {
        flink = new ST_forwardrefs(loc_cnt,size,section,type);
    }
    else{
        ST_forwardrefs* t = new ST_forwardrefs(loc_cnt,size,section,type);
        t->next = flink;
        flink = t;
    }
}
    

std::ostream& operator<< (std::ostream& os, const Symbol& symbol)
{
    int width = 20;
    char sep = ' ';
    string noSection = "no section";

    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << std::hex << symbol.id;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.name;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.value;
    os << left << "|";
    if(symbol.section.compare("")==0)os << left << std::setw(width) << std::setfill(sep) << noSection;
    else os << left << std::setw(width) << std::setfill(sep) << symbol.section;
    string bind = (symbol.binding==1)? "global" : "local";
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << bind;
    os << left << "|";
    os << endl;
    return os;

}
        