#ifndef _SECTION_H_
#define _SECTION_H_

#include <string>
#include <iostream>
#include <iterator>
#include <list>


using namespace std;


class Section
{
    
public:

    static std::list<Section*> globalSectList;
    static void addSectionToGlobalList(Section& section);
    static Section* getSectionByName(string name);

    Section(string name, int s, string input, int addToGlobalList, int fa, int idInOut);

    string getName();
    int getSize();
    string getInputFile();
    std::list<int> getContent();
    int getAddressInOutputFile();
    int isMappedThisSection();
    int getIdInOutputFile();

    void setName(string name);
    void setSize(int size);
    void setInputFile(string file);
    void setContent(list<int> c);
    void setAddressInTheOutputFile(int a);
    void setAsMapped();
    void setIdInOutputFile(int i);

    void addContent(int n, long data);
    void addContentAtSpecificPlace(int n, long data, int where);
    void addContentBigEndian(long data);

    friend ostream& operator<<(ostream& os, const Section& section);

    int inTheOutputFile;

private:

    string name;
    std::list<int> sectionContent;
    int size;
    string inputFile;
    int isMapped;
    int idInOutputFile;

};





#endif