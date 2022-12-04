#include "../inc/relocationrecord.hpp"

#include <string>
#include <iostream>
#include<sstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;

static int staticId = 0;

std::list<RelocationRecord*> RelocationRecord::global_rel_list;

void RelocationRecord::write_table()
{
    int width = 20;
    char sep = ' ';

    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "offset";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "type";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "symbol";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "addend";
    cout << left << "|";
    cout << endl;

    list<RelocationRecord*>::iterator i;
    for (i = global_rel_list.begin(); i != global_rel_list.end(); i++)
    {
        cout << *i;
    }
}

RelocationRecord::RelocationRecord(string section, int offset, RelocationRecordType type, string symbol_name_in_rel_table, int addend)
{
    this->section=section;
    this->offset=offset;
    this->type=type;
    this->symbol_name_in_rel_table = symbol_name_in_rel_table;
    this->addend = addend;
    this->id = staticId++;
    RelocationRecord::add_rel_record_to_list(*this);
}

void RelocationRecord::add_rel_record_to_list(RelocationRecord& rel)
{
    cout << "Dodajem rel zapis #" << rel.get_symbol_name_in_rel_table() << "#........................................." << endl;
    RelocationRecord::global_rel_list.push_back(&rel);
}

RelocationRecord* RelocationRecord::get_rel_record_by_id(int id)
{
    for (auto i = RelocationRecord::global_rel_list.begin(); i != RelocationRecord::global_rel_list.end(); i++)
    {
        RelocationRecord* s = *i;
        if(s->get_id()==id)
        {
            return s;
        }
    }
    return nullptr;
}

string RelocationRecord::get_section()
{
    return this->section;
}

string RelocationRecord::get_symbol_name_in_rel_table()
{
    return this->symbol_name_in_rel_table;
}

RelocationRecordType RelocationRecord::get_type()
{
    return this->type;
}

int RelocationRecord::get_offset()
{
    return this->offset;
}

int RelocationRecord::get_addend()
{
    return this->addend;
}

int RelocationRecord::get_id(){
    return this->id;
}

void RelocationRecord::set_section(string section)
{
    this->section=section;
}

void RelocationRecord::set_symbol_name_in_rel_table(string symbol)
{
    this->symbol_name_in_rel_table=symbol;
}

void RelocationRecord::set_type(RelocationRecordType type)
{
    this->type=type;
}

void RelocationRecord::set_offset(int offset)
{
    this->offset=offset;
}

void RelocationRecord::set_addend(int addend)
{
    this->addend=addend;
}


std::ostream& operator<< (std::ostream& os, const RelocationRecord& symbol)
{
    int width = 20;
    char sep = ' ';

    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << std::hex << symbol.section;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << std::hex << symbol.offset;
    os << left << "|";
    if(symbol.type==R_16) os << left << std::setw(width) << std::setfill(sep) <<  "R_16";
    else os << left << std::setw(width) << std::setfill(sep) <<  "R_16PC";
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.symbol_name_in_rel_table;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << symbol.addend;
    os << left << "|";
    os << endl;
    return os;

}
