#include<fstream>
#include<sstream>
#include<iomanip>
#include<iostream>
#include<iterator>
#include "../inc/linker.hpp"
using namespace std;

int Linker::globalSymbolIds=0;
int Linker::globalSectionIds=0;

Linker::Linker(list<string> sections, list<string> addresses, list<string> files, string fileOut){
    this->firstLineSymbolTable = "Symbol table";
    this->firstLineRelocationRecordTable = "Relocation records";
    this->firstLineSections = "Section content";
    this->regexSymTableString = "([0-9a-fA-F]+)( )*\\|([a-zA-Z]([_a-zA-Z0-9])*|(no section))+( )*\\|([0-9a-fA-F]+)( )*\\|([a-zA-Z]([_a-zA-Z0-9])*|(no section))+( )*\\|(local|global)+( )*\\|";
    this->regexRelTableString= "([a-zA-Z]([_a-zA-Z0-9])*|(no section))+( )*\\|([0-9a-fA-F]+)+( )*\\|(R_16(PC)?)( )*\\|([a-zA-Z]([_a-zA-Z0-9])*|(no section))+( )*\\|(-?[0-9a-fA-F]+)+( )*\\|";
    this->regexSectionNameSizeString = "#([a-zA-Z]([_a-zA-Z0-9])*|(no section))+( )\\(([0-9]+)\\)";
    this->regexSectionContentString = "[0-9a-f]{4}:(( )[0-9a-f]{2}){0,8}";
    this->regexForOneLineSymTable = regex(this->regexSymTableString);
    this->regexForOneLineRelTable = regex(this->regexRelTableString);
    this->regexSectionNameSize = regex(this->regexSectionNameSizeString);
    this->regexSectionContent = regex(this->regexSectionContentString);
    this->data = std::list<int>();
    this->out1 = std::ofstream("myHelpFileLinker.txt");
    this->out1 << "HELP FILE" << endl;
    this->fileOut = std::ofstream(fileOut);

    for(auto i = files.begin(); i !=files.end(); i++){
        string tmp = *i;
        this->listInputFilesNames.push_back(tmp);
    }
}

int Linker::getSizeOfPreviousSections(){
    std::list<Section*>::iterator i;
    int toRet = 0;
    for(i=this->listSectionsInOutputFile.begin();i!=this->listSectionsInOutputFile.end();i++){
        toRet+=(*i)->getSize();
    }
    this->out1 << "SIZE OF PREVIOUS SECTIONS IS " << toRet << endl;
    return toRet;
}

Section* Linker::getSectionByNameAndInputFile(string name, string inputFileName){
    std::list<Section*>::iterator it = listSections.begin();
    for(;it!=listSections.end();it++){
        if((*it)->getName().compare(name)==0 && (*it)->getInputFile().compare(inputFileName)==0){
            return *it;
        }
    }
    return nullptr;
}

Section* Linker::getSectionByName(string name){
    std::list<Section*>::iterator it = listSectionsInOutputFile.begin();
    for(;it!=listSectionsInOutputFile.end();it++){
        if((*it)->getName().compare(name)==0){
            return *it;
        }
    }
    return nullptr;
}

Symbol* Linker::getSymbolByName(string name){
    std::list<Symbol*>::iterator it = listSymbols.begin();
    for(;it!=listSymbols.end();it++){
        if((*it)->get_name().compare(name)==0){
            return *it;
        }
    }
    return nullptr;
}

void Linker::linker(){
    this->readInputFiles();
    this->mapSections();
    this->resolveSymbols();
    this->resolveRelocationRecords();
    std::list<int>::iterator itData;
    int i = 0;
    for(auto itData = data.begin(); itData != data.end(); itData++){
        if(i%16 == 0){
            if(i != 0)this->fileOut << "\n";
            this->fileOut << right << setw(4) << setfill('0') << std::hex << i/2 << ": ";
        }
        this->fileOut << std::hex << *itData;
        if(i%2) this->fileOut << " ";
        i++;
    }
    this->fileOut << endl;
    this->out1 << "\n";
    this->out1 << "END" ;
}

void Linker::readInputFiles(){
    std::list<string>::iterator it;
    cout << "u readInputFiles smo" << endl;
    for(it = this->listInputFilesNames.begin();it!=listInputFilesNames.end();it++){
        this->currentInputFileName = *it;
        this->currentInputFile.open(this->currentInputFileName,std::fstream::in);
        readTables();
        this->currentInputFile.close();
        this->endOfFile = 1;
    }
}

void Linker::readTables(){
    cout << "u readTables smo" << endl;
    if(this->currentInputFile.is_open()){
        cout << "this->currentInputFile.is_open()==1" << endl;
        int end = 0;
        string line0 = "sth";
        string line1 = "sth";
        while(end==0 && line0.compare("")!=0 && line1.compare("")!=0){
            getline(this->currentInputFile,line0);
            this->out1 << "at the beginning of while line0 is " << line0 << endl;
            if(line0.compare(this->firstLineSymbolTable)==0) {
                getline(this->currentInputFile,line1);
                readSymbolTable();
            }
            else if(line0.compare(this->firstLineRelocationRecordTable)==0) {
                getline(this->currentInputFile,line1);
                readRelocationTable();
            }
            else if(line0.compare(this->firstLineSections)==0){
                //getline(this->currentInputFile,line1);
                this->out1 << "line0: " << line0 << ", line1: " << line1 << endl;
                readSections();
                end = 1;
            }
            else{
                end=1;
            }
        }
    }
}

void Linker::readSymbolTable(){
    this->out1 << "u readSymbolTable smo" << endl;
    smatch m;
    int id;
    string idString;
    string name;
    string valueString;
    int value=0;
    string section;
    string bindingString;
    int binding;
    string line;
    getline(this->currentInputFile,line);
    //this->out1 <<"#" << line << "#" << endl;
    while(line!=""){
        if(regex_search(line,m,this->regexForOneLineSymTable)){
            idString = m.str(1);
            id = getNumericalValueFromString(idString,1);
            name = m.str(3);
            valueString = m.str(7);
            value = getNumericalValueFromString(valueString,1);
            section = m.str(9);
            bindingString = m.str(13);
            if(bindingString.compare("local")==0)binding = 0;
            else binding = 1;
            Symbol* sym = new Symbol(id,name,value,section,binding,this->currentInputFileName);
            this->listSymbols.push_back(sym);
            this->out1 << "Symbol addded: id=" << id << ", name=" << name << ", value=" << value << ", section=" << section << ", binding=" << bindingString << endl;
            // for(int i = 0; i<10; i++){
            //     this->out1 << "m.str(" << i <<"): " << m.str(i) << endl;
            // }

        }
        getline(this->currentInputFile,line);
        //this->out1 << "*" << line << "*";
    }
}

void Linker::readRelocationTable(){
    this->out1 << "u readRelocationTable smo" << endl;
    smatch m;
    string section;
    string offsetString;
    int offset;
    string typeString;
    RelocationRecordType type;
    string name;
    string addendString;
    int addend;
    string line;
    getline(this->currentInputFile,line);
    //this->out1 <<"#" << line << "#" << endl;
    while(line!=""){
        if(regex_search(line,m,this->regexForOneLineRelTable)){
            section = m.str(1);
            offsetString = m.str(5);
            offset = getNumericalValueFromString(offsetString,1);
            typeString = m.str(7);
            if(typeString.compare("R_16PC")==0) type = R_16PC;
            else type = R_16;
            name = m.str(10);
            addendString = m.str(14);
            addend = getNumericalValueFromString(addendString,1);
            RelocationRecord* rel = new RelocationRecord(section,offset,type,name,addend,currentInputFileName);
            this->listRelocationRecords.push_back(rel);
            this->out1 << "Relocation record addded: section=" << section << ", offset=" << offset << ", type=" << typeString << ", name=" << name << ", addend=" << addend << endl;
            // for(int i = 0; i<10; i++){
            //     this->out1 << "m.str(" << i <<"): " << m.str(i) << endl;
            // }

        }
        getline(this->currentInputFile,line);
        //this->out1 << "#" << line << "#";
    }
}

void Linker::readSections(){
    this->out1 << "u readSections smo" << endl;
    smatch m;
    string name;
    string sizeString;
    int size;
    string line;
    getline(this->currentInputFile,line);
    while(line!=""){
        this->out1 <<"We checking line #" << line << "#" << endl;
        if(regex_search(line,m,this->regexSectionNameSize)){
            this->out1 <<"It is section and name and size" << endl;
            name = m.str(1);
            sizeString = m.str(5);
            size = getNumericalValueFromString(sizeString,0);
            Section* sec = new Section(name,size,this->currentInputFileName,0,0,-1);
            if(size>0){
                for(int i=0;i<size/8+1;i++){
                    getline(this->currentInputFile,line);
                    this->out1 << i << " :)" << line << ":) " << size << endl;
                    int ind0 = 0;
                    int ind1 = line.find_first_of(':');
                    if(ind1+2 < line.size()) line = line.substr(ind1+2);
                    this->out1 << line << "ind1:" << ind1 << " line.size()" << line.size() << endl;
                    while(ind0!=string::npos && ind0<line.size()){
                        ind0 = line.find_first_of(' ');
                        string toInsert = "";
                        if(2 < line.size()) toInsert = line.substr(0,2);
                        if(ind0+1 <= line.size()) line = line.substr(ind0+1);
                        int data = getNumericalValueFromString(toInsert,1);
                        this->out1 << "toInsert=" << toInsert << " "<< "line=#" << line << "#, line.size()=" << line.size() << ", ind0=" << ind0 << ", INSERTED=";
                        if(data<16){
                            sec->addContent(1,0);
                            sec->addContent(1,data);
                            this->out1 << hex << 0 << data << endl;
                        }
                        else{
                            data = getNumericalValueFromString(toInsert.substr(0,1),1);
                            sec->addContent(1,data);
                            this->out1 << hex << data;
                            data = getNumericalValueFromString(toInsert.substr(1,1),1);
                            sec->addContent(1,data);
                            this->out1 << hex << data;
                        }
                        this->out1 << endl;
                    }
                                        
                }
            }
            this->listSections.push_back(sec);
            std::list<int> t = sec->getContent();
            this->out1 << "Section added, name=" << name << ", size=" << size << ", inputFileName=" << this->currentInputFileName << endl;
            this->out1 << "Section content(" << dec << t.size()/2 << "): ";
            for(auto i = t.begin(); i!=t.end();i++){
                this->out1 << hex << *i << " ";
            }
            this->out1 << endl;
        }
        getline(this->currentInputFile,line);
        this->out1 << "#" << line << "#" << endl;
    }
    addSectionObjectsToSymbolsAndRelocationRecords();
}

void Linker::addSectionObjectsToSymbolsAndRelocationRecords(){
    std::list<Symbol*>::iterator it = listSymbols.begin();
    std::list<RelocationRecord*>::iterator i = listRelocationRecords.begin();
    for(;it!=listSymbols.end();it++){
        Section* sec = getSectionByNameAndInputFile((*it)->get_section(),(*it)->getInputFile());
        (*it)->set_section_object(sec);
    }
    for(;i!=listRelocationRecords.end();i++){
        Section* sec = getSectionByNameAndInputFile((*i)->getSection(),(*i)->getInputFile());
        (*i)->setSectionObject(sec);
    }
}

void Linker::addSectionObjectFromOutputFileToSymbols(){
    std::list<Symbol*>::iterator it = listSymbols.begin();
    for(;it!=listSymbols.end();it++){
        Section* sec = getSectionByName((*it)->get_section());
        (*it)->setSectionInOutputFile(sec);
    }
}

void Linker::mapSections(){
    this->out1 << "WE ARE IN MAPSECTION" << endl;
    std::list<Section*>::iterator i,j,p;
    std::list<int>::iterator m;
    string currName;
    int iter = 0;
    this->out1 << "List of all sections has " << this->listSections.size() << " members: ";
    for(auto m = this->listSections.begin();m!=this->listSections.end();m++){
        this->out1 << (*m)->getName() << "   ";
    }
    this->out1 << endl;


    for(i = this->listSections.begin(), p=this->listSections.begin(); p!=this->listSections.end();p++,iter++){
        i=p;
        Section* s0 = *i;
        currName = s0->getName();
        this->out1 << iter << " WE ARE CHECKING IS NAME " << currName << " ADDED" << endl;
        
        //if section with this name has not added
        //we add it now
        //else we will append this section(it has already added)
        if(isThisSectionNameAdded(currName)==0){
            createNewSectionInOutputFile(currName,s0);
        }
        for(j=++i;j!=this->listSections.end();j++){
            Section* s1 = *j;
            this->out1 << s1->getName() << " == " << currName << "? " << currName.compare(s1->getName()) << endl;
            if(currName.compare(s1->getName())==0 && s1->getSize()>0 && s1->isMappedThisSection()==0){
                this->out1 << "yes, we APPEND section " << currName << " with content from " << (*j)->getInputFile() << endl;
                Section* secNew = Section::getSectionByName(currName);
                appendSection(secNew,s1);
            }
            else{
                this->out1 << "No or second section are empty" << endl;
            }
        }
    }
    this->out1 << endl << "Before mapping sections:" << endl;
    for(i = this->listSections.begin(); i!=this->listSections.end();i++){
        Section* sec = *i;
        std::list<int> t = sec->getContent();
        this->out1 << sec->getName() << ", section content(" << dec << t.size()/2 << "): ";
        this->out1 << *sec;
    }
    this->out1 << "After mapping sections:"  << endl;
    for(i = this->listSectionsInOutputFile.begin(); i!=this->listSectionsInOutputFile.end();i++){
        Section* sec = *i;
        std::list<int> t = sec->getContent();
        this->out1 << sec->getName() << ", section content(" << dec << t.size()/2 << "): ";
        // for(auto i = t.begin(); i!=t.end();i++){
        //     this->out1 << hex << *i << " ";
        // }
        // this->out1 << endl;
        this->out1 << *sec;
    }
    this->out1  << endl << endl << "After mapping symbols:" << endl;
    std::list<Symbol*>::iterator it;
    for(it=this->listSymbols.begin();it!=this->listSymbols.end();it++){
        this->out1 << **it;
    }

    addSectionObjectFromOutputFileToSymbols();
    this->out1<< "\nWe went out from mapping sections" << endl;
}

void Linker::appendSection(Section* secToExpend, Section* secToAdd){
    std::list<int>::iterator m;
    int s = getSizeOfPreviousSections();
    this->out1 << "Size of previous sections: " << s << endl;
    secToAdd->setAddressInTheOutputFile(s);
    secToAdd->setAsMapped();
    updateTheStartingAddressesOfAlreadyMappedSections(secToAdd->getSize(),secToAdd,secToExpend);
    this->out1 << "FIRST ADDRESS OF SECTION " << secToAdd->getName() << " FROM INPUT FILE " << secToAdd->getInputFile() << " IS " << secToAdd->getAddressInOutputFile() << endl;
    list<int> list0 = secToExpend->getContent();
    list<int> list1 = secToAdd->getContent();
    int size0 = list0.size();
    int size1 = list1.size();
    this->out1 << "list0: ";
    for(auto q = list0.begin();q!=list0.end();q++){
        this->out1 << *q << " ";
    }
    this->out1 << endl;
    this->out1 << "list1: ";
    for(auto q = list1.begin();q!=list1.end();q++){
        this->out1 << *q << " ";
    }
    list0.resize(size0+size1,0);
    this->out1 << "list0(after resizing): ";
    for(auto q = list0.begin();q!=list0.end();q++){
        this->out1 << *q << " ";
    }
    m = list0.begin();
    std::advance(m,size0);
    list0.splice(m,list1);
    this->out1 << "list0(after splicing): ";
    for(auto q = list0.begin();q!=list0.end();q++){
        this->out1 << *q << " ";
    }
    this->out1 << endl;
    list0.resize(size0+size1);
    this->out1 << "finally: ";
    for(auto q = list0.begin();q!=list0.end();q++){
        this->out1 << *q << " ";
    }
    this->out1 << endl;
    secToExpend->setSize((size0+size1)/2);
    secToExpend->setContent(list0);
    this->out1 << "SECTION " << secToAdd->getName() << " IS APPEND, NEW SIZE IS " << (size0+size1)/2 << endl;
}

void Linker::createNewSectionInOutputFile(string nameOfSection, Section* sec){
        int s = this->getSizeOfPreviousSections();
        Section* secNew = new Section(nameOfSection,sec->getSize(),"outFile",1,s,globalSectionIds++);
        secNew->setContent(sec->getContent());
        secNew->setAddressInTheOutputFile(s);
        secNew->setAsMapped();
        sec->setAddressInTheOutputFile(s);
        this->listSectionsInOutputFile.push_back(secNew);
        updateTheStartingAddressesOfAlreadyMappedSections(sec->getSize(),sec,secNew);
        this->out1 << "SECTION " << nameOfSection << " ADDED TO LIST OF MAPPED SECTIONS, FIRST ADDRESS IS " << s << endl;
}

void Linker::updateTheStartingAddressesOfAlreadyMappedSections(int sizeOfNewOne, Section* newOneOldSections, Section* newOneNewSections){
    //we have to update all section which are after specific section
    std::list<Section*>::iterator it= listSections.begin();
    int flag = 0;
    for(;it!=listSections.end();it++){
        this->out1 << (*it)->getName() << " from " << (*it)->getInputFile() << endl;
        if((*it)->isMappedThisSection() && flag==1){
            this->out1 << "we INCREASE size of section " << (*it)->getName() << " from " << (*it)->getInputFile() << " for " << sizeOfNewOne << endl;
            (*it)->setAddressInTheOutputFile((*it)->getAddressInOutputFile()+sizeOfNewOne);
        }
        if((*it)->getName().compare(newOneOldSections->getName())==0 && (*it)->getInputFile().compare(newOneOldSections->getInputFile())
        && (*it)->getAddressInOutputFile()==newOneOldSections->getAddressInOutputFile()){
            flag = 1;
        }
    }
    flag=0;
    for(it=listSectionsInOutputFile.begin();it!=listSectionsInOutputFile.end();it++){
        this->out1 << (*it)->getName() << " from " << (*it)->getInputFile() << endl;
        if(flag==1){
            this->out1 << "we INCREASE size of section " << (*it)->getName() << " from " << (*it)->getInputFile() << " for " << sizeOfNewOne << endl;
            (*it)->setAddressInTheOutputFile((*it)->getAddressInOutputFile()+sizeOfNewOne);
        }
        if((*it)->getName().compare(newOneNewSections->getName())==0 && (*it)->getInputFile().compare(newOneNewSections->getInputFile())
        && (*it)->getAddressInOutputFile()==newOneNewSections->getAddressInOutputFile()){
            flag = 1;
        }
    }
}

void Linker::solveTheSymbols(){
    // undefined - stop
    // multiple defined - choose one definition
    // multiple strong symbols are not allowed (functions and initialized global symbols)
    // weak symbols are undefined global symbols


    //GLOBAL SYMBOL - value = oldValue+addrInOutputFile(section where sym is defined)
    // you should change and section object

    //offset in rel. records is old offset + addrInOutputFile of section which record belongs
    //add to addend NEW value of symbol(global), NEW value of section(local)
}

void Linker::resolveSymbols(){
    std::list<Symbol*>::iterator iterSymbols;
    for(iterSymbols=listSymbols.begin();iterSymbols!=listSymbols.end();iterSymbols++){
        if((*iterSymbols)->get_name().compare((*iterSymbols)->get_section())==0){
            Symbol* sym = *iterSymbols;
            Section* sec = sym->get_section_object();
            string in = "";
            int newVal = 0;
            if(sec!=nullptr) {
                newVal = sec->getAddressInOutputFile();
                in=sec->getInputFile();
            }
            this->out1 << "In finally file section " << sym->get_name() << " from file " << in << " is at offset " << newVal << endl;
            sym->set_value(newVal);
        }
        else{
            Symbol* sym = *iterSymbols;
            Section* sec = sym->get_section_object();
            string in = "";
            int newVal = sym->get_value();
            if(sec!=nullptr) {
                newVal += sec->getAddressInOutputFile();
                in=sec->getInputFile();
            }
            this->out1 << "In finally file symbol " << sym->get_name() << " from file " << in << " has value " << newVal << endl;
            sym->set_value(newVal);
        }
    }
    filterSymbols();
    this->out1 << "New symbol table(after resolving symbols): " << endl;
    for(iterSymbols=this->listSymbols.begin();iterSymbols!=this->listSymbols.end();iterSymbols++){
        this->out1 << **iterSymbols;
    }
}

void Linker::resolveRelocationRecords(){
    generateContent();
    std::list<RelocationRecord*>::iterator it0;
    std::list<int> listToAdd = std::list<int>();
    std::list<int>::iterator it1;
    int i = 0, valToAdd=0, placeOfModification=0;
    unsigned firstToAdd=0, secondToAdd=0;

    for(it0=listRelocationRecords.begin();it0!=listRelocationRecords.end();it0++){
        Section* sec = (*it0)->getSectionObject();
        placeOfModification = sec->getAddressInOutputFile() + (*it0)->getOffset();
        
        if((*it0)->getType()==R_16){
            valToAdd = getSymbolByName((*it0)->getName())->get_value() + (*it0)->getAddend();
        }
        else{
            Symbol* sym = getSymbolByName((*it0)->getName());
            if(sym->is_global()==1){
                valToAdd = sym->get_value() + (*it0)->getAddend() - placeOfModification;
            }
            else{
                valToAdd = sym->get_section_object()->getAddressInOutputFile() + (*it0)->getAddend() - placeOfModification;
            }
        }
        firstToAdd = valToAdd & 0xFF;
        secondToAdd = (valToAdd>>8) & 0xFF;
        addContentAtSpecificPlace(1,firstToAdd,placeOfModification);
        addContentAtSpecificPlace(1,secondToAdd,placeOfModification+1);

        this->out1 << "Data in output file after resolving relocation record for symbol " << (*it0)->getName();
        this->out1 << ", value to add " << valToAdd << ", at address " << placeOfModification << endl;
        printData();
            
    }
}

void Linker::generateContent(){
    std::list<Section*>::iterator it;
    std::list<int>::iterator itData;
    for(it=listSectionsInOutputFile.begin();it!=listSectionsInOutputFile.end();it++){
        itData = data.end();
        data.splice(itData,(*it)->getContent());
    }
    this->out1 << "Data in output file before resolving relocation records: " << endl;
    printData();
}

void Linker::printData(){
    std::list<int>::iterator itData;
    int i = 0;
    for(auto itData = data.begin(); itData != data.end(); itData++){
        if( i%16 == 0){
            this->out1 << "\n";
            this->out1 << right << setw(4) << setfill('0') << std::hex << i/2 << ": ";
        }
        this->out1 << std::hex << *itData;
        if(i%2) this->out1 << " ";
        i++;
    }
    this->out1 << "\n";
}

void Linker::addContentAtSpecificPlace(int n, long data, int where){
    // erase(i) - remove element from position i in list
    // splice(i) - add element at a position i in list
    
    where=where*2;
    if(n > this->data.size() || (where + n) > this->data.size()) return;
    std::list<int>::iterator it1, it11,it2, it22;
    std::list<int> mylist;
    int secondDigit = data % 16;
    int firstDigit = data / 16;
    this->out1 << "adding val " << data << " at address " << where/2 << " (first=" << firstDigit << ",second=" << secondDigit << ")" << endl;

    if(n==1){
        it1 = this->data.begin();
        std::advance(it1,where);
        this->data.erase(it1);
        it11 = this->data.begin();
        std::advance(it11,where);
        this->data.erase(it11);
        this->out1 << "Data after removing value " << *it1 << *it11<< " from address(offset) " << where/2 << "(" << hex << where/2 << ")" << endl; 
        printData();
        // mylist.push_back(data);
        mylist.push_back(firstDigit);
        mylist.push_back(secondDigit);
        it2 = this->data.begin();
        std::advance(it2,where);
        this->data.splice(it2,mylist);
        this->out1 << "Data after inserting value " << firstDigit << secondDigit << " at address(offset) " << where/2 << "(" << hex << where/2 << ")" << endl; 
        printData();
    }
    else {
        for(int i = 0; i < n; i++){
            it1 = this->data.begin();
            std::advance(it1,where);
            this->data.erase(it1);
            mylist.push_back((data >> 8*i) & 0xFF);
            int j = (data >> 8*i) & 0xFF;
            cout <<"n=" << n << " stavio sam u sekciju val: " << j << " na mesto: " << where << endl;
        }
        it2 = this->data.begin();
        std::advance(it2,where);
        this->data.splice(it2,mylist);
    }
}

void Linker::filterSymbols(){
    std::list<Symbol*> listSymbolsToAdd = std::list<Symbol*>();
    std::list<Symbol*>::iterator itSym;
    std::list<Section*>::iterator itSec;
    int isDefined = 0;
    int multipleDefined = 0;
    int value = 0;
    Symbol* definedSymbol;
    for(itSec=listSectionsInOutputFile.begin();itSec!=listSectionsInOutputFile.end();itSec++){
        this->out1 << "globalSymbolIds="<<globalSymbolIds << endl;
        Symbol* symNew = new Symbol(globalSymbolIds++,(*itSec)->getName(),(*itSec)->getAddressInOutputFile(),(*itSec)->getName(),0,"outputFile");
        listSymbolsToAdd.push_back(symNew);
        listSymbols = removeSymbolsWithSpecificName(symNew->get_name());
    }
    for(itSym=listSymbols.begin();itSym!=listSymbols.end();itSym++){
        Symbol* tmp = *itSym;
        isDefined = checkIsDefined(tmp);
        multipleDefined = checkMultipleDefinitions(tmp);
        int isAlreadyAdded = isSymbolInList(listSymbolsToAdd,tmp->get_name());
        this->out1 << "symbol " << tmp->get_name() << " defined? " << isDefined << " multipleDefined? " << multipleDefined << endl;
        if(isDefined==1 && multipleDefined==0 && isAlreadyAdded==0){
            Symbol* symNew = getDefinedSymbol(tmp->get_name());
            if(symNew!=nullptr){
                this->out1 << "globalSymbolIds="<<globalSymbolIds << endl;
                symNew->set_id(globalSymbolIds++);
                listSymbolsToAdd.push_back(symNew);
            }
        }
    }
    this->out1 << "List of symbols to add: " << endl;
    for(itSym=listSymbolsToAdd.begin();itSym!=listSymbolsToAdd.end();itSym++){
        this->out1 << (*itSym)->get_name() << " ";
    }
    this->out1  << endl;
    for(itSym=listSymbolsToAdd.begin();itSym!=listSymbolsToAdd.end();itSym++){
        Symbol* tmp = *itSym;
        listSymbols = removeSymbolsWithSpecificName(tmp->get_name());
        listSymbols.push_back(tmp);
        this->out1 << "Symbol " << tmp->get_name() << " added, id: " << tmp->get_id() << endl;
    }
}

int Linker::isSymbolInList(std::list<Symbol*> whichList, string name){
    std::list<Symbol*>::iterator it;
    for(it=whichList.begin();it!=whichList.end();it++){
        if((*it)->get_name().compare(name)==0) return 1;
    }
    return 0;
}

int Linker::checkMultipleDefinitions(Symbol* symbol){
    string name = symbol->get_name();
    int value = symbol->get_value();
    std::list<Symbol*>::iterator it;
    int isDefined = 0;
    int multipleDefined = 0;
    for(it=listSymbols.begin();it!=listSymbols.end();it++){
        Symbol* tmp = *it;
        if(tmp->get_name().compare(tmp->get_section())!=0 && tmp->get_name().compare(name)==0 && tmp->get_section().compare("no section")!=0 && isDefined==1 && tmp->get_value()!=value){
            multipleDefined = 1;
            this->out1 << "******************************************************************************************" << endl;
            this->out1 << "SYMBOL " << tmp->get_name() << " IS MULTIPLE DEFINED. (in file " << symbol->getInputFile() << " has value " << symbol->get_value();
            this->out1 << ", in file " << tmp->getInputFile() << " has value " << tmp->get_value() << ")" << endl;
            this->out1 << "******************************************************************************************" << endl;
        }
        else if(tmp->get_name().compare(name)==0 && tmp->get_section().compare("no section")!=0 && isDefined==0){
            isDefined = 1;
            value = tmp->get_value();
        }
    }
    return multipleDefined;
}

int Linker::checkIsDefined(Symbol* symbol){
    string name = symbol->get_name();
    std::list<Symbol*>::iterator it;
    int isDefined = 0;
    for(it=listSymbols.begin();it!=listSymbols.end();it++){
        Symbol* tmp = *it;
        if(tmp->get_name().compare(name)==0 && tmp->get_section().compare("no section")!=0 && isDefined==0){
            isDefined = 1;
        }
    }
    return isDefined;
}

Symbol* Linker::getDefinedSymbol(string name){
    std::list<Symbol*>::iterator it;
    for(it=listSymbols.begin();it!=listSymbols.end();it++){
        Symbol* tmp = *it;
        if(tmp->get_name().compare(name)==0 && tmp->get_section().compare("no section")!=0 ){
            return tmp;
        }
    }
    return nullptr;
}

std::list<Symbol*> Linker::removeSymbolsWithSpecificName(string name){
    std::list<Symbol*> listToRet = std::list<Symbol*>();
    std::list<Symbol*>::iterator it;
    this->out1 << "Removing symbol " << name << " from listSymbols";
    for(it=listSymbols.begin();it!=listSymbols.end();it++){
        if(name.compare((*it)->get_name())!=0){
            listToRet.push_back((*it));
        }
    }
    this->out1 << "(removed)"<< endl;
    return listToRet;
}

int Linker::isThisSectionNameAdded(string name){
    for(auto i = this->listSectionsInOutputFile.begin();i!=this->listSectionsInOutputFile.end();i++){
        Section* s = *i;
        if(name.compare(s->getName())==0) return 1;
    }
    return 0;
}

int Linker::getNumericalValueFromString(string line, int isHex){

    smatch m;
    regex regex_literal = regex("([0-9]+)|(([0-9a-fA-F])+)");
    if(regex_search(line,m,regex_literal) == false) return INT32_MIN;
    
    //cout << line + " is line"<< endl; 
    int x=-1;
    if(isHex){
        x=std::stoul(line,nullptr,16);
    }
    else{
        x=std::stoul(line,nullptr,10);
    }
    //this->out1 << "numerical value from string '" << line << "' is: " << hex << x  << " (" << dec << x << ")" << endl;
    return x;
}