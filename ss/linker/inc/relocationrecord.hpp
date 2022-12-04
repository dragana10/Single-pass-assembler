#ifndef RELOCATION_RECORD_
#define RELOCATION_RECORD_

#include <string>
#include <iostream>
#include <iterator>
#include <list>

#include "section.hpp"

using namespace std;

enum RelocationRecordType{
    R_16,
    R_16PC
};


class RelocationRecord
{

public:

    static std::list<RelocationRecord*> global_rel_list;

    RelocationRecord(string section, int offset, RelocationRecordType type, string symbol, int addend, string inputF);
    static RelocationRecord* getRelRecordById(int id);

    string getSection();
    string getName();
    RelocationRecordType getType();
    int getOffset();
    int getAddend();
    int getId();
    string getInputFile();
    Section* getSectionObject();

    void setSection(string section);
    void setName(string symbol);
    void setType(RelocationRecordType type);
    void setOffset(int offset);
    void setAddend(int addend);
    void addReRecordToList(RelocationRecord& rel);
    void setInputFile(string name);
    void setSectionObject(Section* sec);

    friend ostream& operator<<(ostream& os, const RelocationRecord& rt);

private:

    string section;
    int offset;
    RelocationRecordType type;
    string name;
    int addend;
    int id;
    string inputFile;
    Section* sectionObject;

};


#endif