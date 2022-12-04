#include "../inc/section.hpp"

#include <string>
#include <iostream>
#include<sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <list>
#include <math.h>

using namespace std;

std::list<Section*> Section::globalSectList;

void Section::addSectionToGlobalList(Section& section)
{
    globalSectList.push_back(&section);
}

Section* Section::getSectionByName(string name){
    if(globalSectList.size()>0){
        for (auto i = globalSectList.begin(); i != globalSectList.end(); i++)
        {
            Section* s = *i;
            if((s->name.compare(name))==0)
            {
                return s;
            }
        }
    }
    return nullptr;
}

Section::Section(string name, int s, string input, int addToGlobalList, int fa, int idInOut)
{
    this->name=name;
    this->inputFile = input;
    this->size=s;
    this->sectionContent=list<int>();
    this->inTheOutputFile = fa;
    this->isMapped = 0;
    this->idInOutputFile = idInOut;
    if(addToGlobalList)addSectionToGlobalList(*this);
}

string Section::getName()
{
    return this->name;
}

int Section::getSize()
{
    return this->size;
}

string Section::getInputFile()
{
    return this->inputFile;
}

std::list<int> Section::getContent(){
    return this->sectionContent;
}

int Section::getAddressInOutputFile(){
    return this->inTheOutputFile;
}

int Section::isMappedThisSection(){
    return this->isMapped;
}

int Section::getIdInOutputFile(){
    return this->idInOutputFile;
}

void Section::setName(string name)
{
    this->name=name;
}

void Section::setSize(int size)
{
    this->size=size;
}

void Section::setInputFile(string file)
{
    this->inputFile=file;
}

void Section::setContent(list<int> c){
    this->sectionContent = c;
}

void Section::setAddressInTheOutputFile(int a){
    this->inTheOutputFile = a;
}

void Section::setAsMapped(){
    this->isMapped=1;
}

void Section::setIdInOutputFile(int i){
    this->idInOutputFile = i;
}

void Section::addContent(int n, long data){
    // data are stored on little-endian principle
    if(n==1){
        this->sectionContent.push_back(data);
    }
    else if(n==2){
        this->sectionContent.push_back((data >> 8) & 0xFF);
        this->sectionContent.push_back(data & 0xFF);
    }
    else if(n==3){
        this->sectionContent.push_back((data >> 16) & 0xFF);
        this->sectionContent.push_back((data >> 8) & 0xFF);
        this->sectionContent.push_back(data & 0xFF);
    }
    else if(n==4){
        this->sectionContent.push_back((data >> 24) & 0xFF);
        this->sectionContent.push_back((data >> 16) & 0xFF);
        this->sectionContent.push_back((data >> 8) & 0xFF);
        this->sectionContent.push_back(data & 0xFF);
    }
    else if(n==5){
        this->sectionContent.push_back((data >> 32) & 0xFF);
        this->sectionContent.push_back((data >> 24) & 0xFF);
        this->sectionContent.push_back((data >> 16) & 0xFF);
        this->sectionContent.push_back((data >> 8) & 0xFF);
        this->sectionContent.push_back(data & 0xFF);
    }

}

void Section::addContentBigEndian(long data){
    //big-endian principle
    this->sectionContent.push_back(data);
}

void Section::addContentAtSpecificPlace(int n, long data, int where){
    // erase(i) - remove element from position i in list
    // splice(i) - add element at a position i in list

    if(n > this->sectionContent.size() || (where + n) > this->sectionContent.size()) return;
    std::list<int>::iterator it1,it2;
    std::list<int> mylist;

    if(n==1){
        it1 = this->sectionContent.begin();
        std::advance(it1,where);
        this->sectionContent.erase(it1);
        mylist.push_back(data);
        it2 = this->sectionContent.begin();
        std::advance(it2,where);
        this->sectionContent.splice(it2,mylist);
        cout <<"n=" << n << " stavio sam u sekciju val: " << data << " na mesto: " << where << endl;
    }
    else {
        for(int i = 0; i < n; i++){
            it1 = this->sectionContent.begin();
            std::advance(it1,where);
            this->sectionContent.erase(it1);
            mylist.push_back((data >> 8*i) & 0xFF);
            int j = (data >> 8*i) & 0xFF;
            cout <<"n=" << n << " stavio sam u sekciju val: " << j << " na mesto: " << where << endl;
        }
        it2 = this->sectionContent.begin();
        std::advance(it2,where);
        this->sectionContent.splice(it2,mylist);
    }
}

ostream& operator<<(ostream& os, const Section& section)
{
    os << "#" << section.name << " (" << to_string(section.size) << ") newOffset: " << section.inTheOutputFile;

    int i = 0;
    for(auto it = section.sectionContent.begin(); it != section.sectionContent.end(); it++){
        if( i%16 == 0){
            os << "\n";
            os << right << setw(4) << setfill('0') << std::hex << i/2 << ": ";
        }
        os << std::hex << *it;
        if(i%2) os << " ";
        i++;
    }
    os << "\n";
    return os;
}