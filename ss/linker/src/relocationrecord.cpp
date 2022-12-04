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


RelocationRecord::RelocationRecord(string section, int offset, RelocationRecordType type, string symbol_name_in_rel_table, int addend, string inputF)
{
    this->section=section;
    this->offset=offset;
    this->type=type;
    this->name = symbol_name_in_rel_table;
    this->addend = addend;
    this->id = staticId++;
    this->inputFile = inputF;
    RelocationRecord::addReRecordToList(*this);
}

void RelocationRecord::addReRecordToList(RelocationRecord& rel)
{
    RelocationRecord::global_rel_list.push_back(&rel);
}

RelocationRecord* RelocationRecord::getRelRecordById(int id)
{
    for (auto i = RelocationRecord::global_rel_list.begin(); i != RelocationRecord::global_rel_list.end(); i++)
    {
        RelocationRecord* s = *i;
        if(s->getId()==id)
        {
            return s;
        }
    }
    return nullptr;
}

string RelocationRecord::getSection()
{
    return this->section;
}

string RelocationRecord::getName()
{
    return this->name;
}

RelocationRecordType RelocationRecord::getType()
{
    return this->type;
}

int RelocationRecord::getOffset()
{
    return this->offset;
}

int RelocationRecord::getAddend()
{
    return this->addend;
}

int RelocationRecord::getId(){
    return this->id;
}

string RelocationRecord::getInputFile(){
    return this->inputFile;
}

Section* RelocationRecord::getSectionObject(){
    return this->sectionObject;
}

void RelocationRecord::setSection(string section)
{
    this->section=section;
}

void RelocationRecord::setName(string symbol)
{
    this->name=symbol;
}

void RelocationRecord::setType(RelocationRecordType type)
{
    this->type=type;
}

void RelocationRecord::setOffset(int offset)
{
    this->offset=offset;
}

void RelocationRecord::setAddend(int addend)
{
    this->addend=addend;
}

void RelocationRecord::setInputFile(string name){
    this->inputFile=name;
}

void RelocationRecord::setSectionObject(Section* sec){
    this->sectionObject=sec;
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
    os << left << std::setw(width) << std::setfill(sep) <<  symbol.name;
    os << left << "|";
    os << left << std::setw(width) << std::setfill(sep) << symbol.addend;
    os << left << "|";
    os << endl;
    return os;

}
