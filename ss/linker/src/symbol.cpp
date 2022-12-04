#include "../inc/symbol.hpp"

#include <string>
#include<sstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;
std::list<Symbol*> Symbol::global_sym_list;


Symbol::Symbol(int id, std::string name, int value, std::string section, int binding, string input)
{
    this->id = id;
    this->name=name;
    this->value=value;
    this->section=section;
    this->binding=binding;
    this->inputFile = input;
    Symbol::add_symbol_to_list(*this);
}

void Symbol::add_symbol_to_list(Symbol& symbol)
{
    Symbol::global_sym_list.push_back(&symbol);
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

string Symbol::get_section()
{
    return this->section;
}

Section* Symbol::get_section_object()
{
    return this->section_object;
}

int Symbol::is_global()
{
    return this->binding;
}

string Symbol::getInputFile(){
    return this->inputFile;
}

Section* Symbol::getSectionInOutputFile(){
    return this->sectionInOutputFile;
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

void Symbol::setSectionInOutputFile(Section* s){
    this->sectionInOutputFile = s;
} 

std::ostream& operator<< (std::ostream& os, const Symbol& symbol)
{
    int width = 13;
    char sep = ' ';
    string noSection = "no section";
    Section* sec = symbol.section_object;

    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << std::hex << symbol.id;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.name;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.value;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << symbol.section;
    string bind = (symbol.binding==1)? "global" : "local";
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << bind;
    os << left << "|";
    os << left << std::setw(22) << std::setfill(sep) << symbol.inputFile;
    os << left << "|";
    os << endl;
    return os;

}
        