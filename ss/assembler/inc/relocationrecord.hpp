#ifndef _RELOCATION_RECORD_
#define _RELOCATION_RECORD_

#include <string>
#include <iostream>
#include <iterator>
#include <list>

using namespace std;

enum RelocationRecordType{
    R_16,
    R_16PC
};


class RelocationRecord
{

public:

    static std::list<RelocationRecord*> global_rel_list;
    static void write_table();

    RelocationRecord(string section, int offset, RelocationRecordType type, string symbol, int addend);
    static RelocationRecord* get_rel_record_by_id(int id);

    string get_section();
    string get_symbol_name_in_rel_table();
    RelocationRecordType get_type();
    int get_offset();
    int get_addend();
    int get_id();

    void set_section(string section);
    void set_symbol_name_in_rel_table(string symbol);
    void set_type(RelocationRecordType type);
    void set_offset(int offset);
    void set_addend(int addend);
    void add_rel_record_to_list(RelocationRecord& rel);

    friend ostream& operator<<(ostream& os, const RelocationRecord& rt);

private:

    string section;
    int offset;
    RelocationRecordType type;
    string symbol_name_in_rel_table;
    int addend;
    int id;

};


#endif