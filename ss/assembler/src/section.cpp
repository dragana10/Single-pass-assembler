#include "../inc/section.hpp"
#include "../inc/relocationrecord.hpp"

#include <string>
#include <iostream>
#include<sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <list>
#include <math.h>

using namespace std;

std::list<Section*> Section::global_sect_list;

void Section::add_section_to_global_list(Section& section)
{
    global_sect_list.push_back(&section);
}

Section* Section::get_section_by_name(string name){
    for (auto i = Section::global_sect_list.begin(); i != Section::global_sect_list.end(); i++)
    {
        Section* s = *i;
        if((s->name.compare(name))==0)
        {
            return s;
        }
    }
    return nullptr;
}

Section::Section(string name)
{
    this->name=name;
    this->size=0;
    this->data="";
    this->relocation_table = list<RelocationRecord*>();
    this->section_content=list<int>();
    add_section_to_global_list(*this);
}

string Section::get_name()
{
    return this->name;
}

int Section::get_size()
{
    return this->size;
}

void Section::set_name(string name)
{
    this->name=name;
}

void Section::set_size(int size)
{
    this->size=size;
}

void Section::add_relocation_record(RelocationRecord& rel){
    cout << "DODAT REL ZAPIS ZA SEKCIJU: " << rel.get_section() << endl;
    this->list_of_my_relocation_records.push_back(&rel);
}


// RelocationTable Section::get_relocation_table()
// {
    
// }

void Section::add_content(int n, long data){
    // data are stored on little-endian principle
    if(n==1){
        this->section_content.push_back(data);
    }
    else if(n==2){
        this->section_content.push_back((data >> 8) & 0xFF);
        this->section_content.push_back(data & 0xFF);
    }
    else if(n==3){
        this->section_content.push_back((data >> 16) & 0xFF);
        this->section_content.push_back((data >> 8) & 0xFF);
        this->section_content.push_back(data & 0xFF);
    }
    else if(n==4){
        this->section_content.push_back((data >> 24) & 0xFF);
        this->section_content.push_back((data >> 16) & 0xFF);
        this->section_content.push_back((data >> 8) & 0xFF);
        this->section_content.push_back(data & 0xFF);
    }
    else if(n==5){
        this->section_content.push_back((data >> 32) & 0xFF);
        this->section_content.push_back((data >> 24) & 0xFF);
        this->section_content.push_back((data >> 16) & 0xFF);
        this->section_content.push_back((data >> 8) & 0xFF);
        this->section_content.push_back(data & 0xFF);
    }

}

void Section::add_content_big_endian(long data){
    //big-endian principle
    this->section_content.push_back(data);
}

void Section::add_content_at_specific_place(int n, long data, int where){
    // erase(i) - remove element from position i in list
    // splice(i) - add element at a position i in list

    if(n > this->section_content.size() || (where + n) > this->section_content.size()) return;
    std::list<int>::iterator it1,it2;
    std::list<int> mylist;

    if(n==1){
        it1 = this->section_content.begin();
        std::advance(it1,where);
        this->section_content.erase(it1);
        mylist.push_back(data);
        it2 = this->section_content.begin();
        std::advance(it2,where);
        this->section_content.splice(it2,mylist);
        cout <<"n=" << n << " stavio sam u sekciju val: " << data << " na mesto: " << where << endl;
    }
    else {
        for(int i = 0; i < n; i++){
            it1 = this->section_content.begin();
            std::advance(it1,where);
            this->section_content.erase(it1);
            mylist.push_back((data >> 8*i) & 0xFF);
            int j = (data >> 8*i) & 0xFF;
            cout <<"n=" << n << " stavio sam u sekciju val: " << j << " na mesto: " << where << endl;
        }
        it2 = this->section_content.begin();
        std::advance(it2,where);
        this->section_content.splice(it2,mylist);
    }
}

ostream& operator<<(ostream& os, const Section& section)
{
    os << "#" << section.name << " (" << to_string(section.size) << ")";

    int i = 0;
    for(auto it = section.section_content.begin(); it != section.section_content.end(); it++){
        if( i%8 == 0){
            os << "\n";
            os << right << setw(4) << setfill('0') << std::hex << i << ": ";
        }
        if(*it < 16) os << "0";
        os << std::hex << *it << " ";
        i++;
    }
    os << "\n";
    return os;
}