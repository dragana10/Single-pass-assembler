#ifndef _SECTION_H_
#define _SECTION_H_

#include <string>
#include <iostream>
#include <iterator>
#include <list>

#include "relocationrecord.hpp"

using namespace std;

#define SECTION_NOT_DEF "not def"
#define SECTION_DATA "data"
#define SECTION_TEXT "text"
#define SECTION_RODATA "rodata"
#define SECTION_BSS "bss"

class Section
{
    
public:

    static std::list<Section*> global_sect_list;
    static void add_section_to_global_list(Section& section);
    static Section* get_section_by_name(string name);

    Section(string name);

    string get_name();
    int get_size();

    void set_name(string name);
    void set_size(int size);

    void add_content(int n, long data);
    void add_content_at_specific_place(int n, long data, int where);
    void add_content_big_endian(long data);

    void add_relocation_record(RelocationRecord& rel);
    // RelocationRecord get_relocation_table();


    std::list<RelocationRecord*> list_of_my_relocation_records;
    friend ostream& operator<<(ostream& os, const Section& section);


private:

    string name;
    string data;
    std::list<int> section_content;
    int size;
    std::list<RelocationRecord*> relocation_table;

};





#endif