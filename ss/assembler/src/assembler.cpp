#include "../inc/assembler.hpp"
#include "../inc/relocationrecord.hpp"
#include "../inc/section.hpp"
#include "../inc/symbol.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <list>
#include <math.h>

using namespace std;

#define NO_SECTION "no section"
#define ABS_SECTION "absolute"
#define ERR_NONE "***************************************************NO ERRORS******************************************************"
#define ERR_NO_SEC "****************************************SYMBOL IS NOT A PART OF ANY SECTION***************************************"
#define ERR_NO_DIR_OR_INS "***********************************THERE IS UNKNOWN DIRECTIVE OR INSTRUCTION**************************************"
#define ERR_MULTIPLE_DEF "**********************************************MULTIPLE DEFINITION*************************************************"
#define ERR_DEF_LAB_IN_SEC "***********************************LABEL HAS ALREADY DEFINED IN THIS SECTION**************************************"
#define ERR_REDEF_EX_SYM "*******************************************REDEFINED EXTERNAL SYMBOL**********************************************"
#define ERR_UNKNOWN_COMMAND "************************************************UNKNOWN COMMAND***************************************************"
#define ERR_UNDEF_GLOB_SYM "******************************************THE GLOBAL SYMBOL IS NOT DEFINED****************************************"
#define ERR_SYM_IS_NOT_IN_TABLE "********************************************SYMBOL IS NOT IN SYMBOL TABLE*****************************************"
#define ERR_ASCII_PART_OF_BSS "******************************THE .ASCII DIRECTIVE IS NOT VALID FOR THE .BSS SECTION******************************"

Assembler::Assembler(string file_in, string file_out)
{

    #pragma region AUXILIARY_REGEX_EXPRESSIONS

    // auxiliary basic patterns
    this->reg_sym = "[a-zA-Z]([_a-zA-Z0-9])*";
    this->reg_num = "-?[0-9]+";
    this->reg_hex = "(0x([0-9A-F])+)+";
    this->reg_ascii = "[ -~]";
    //this->reg_lit = "(" + this->reg_num + ")|(" + this->reg_hex + ")";
    this->reg_lit = "(-?[0-9]+)|((0x([0-9A-F])+)+)";
    this->reg_sym_lit = "(" + this->reg_sym + ")|(" + this->reg_lit + ")";
    this->reg_sym_list = "(" + this->reg_sym + "(( )*,( )*" + this->reg_sym + ")*)";
    // check this lit_list
    this->reg_lit_list = "(" + this->reg_lit + "(( )*,( )*" + this->reg_lit + ")*)";
    this->reg_sym_lit_list = "(" + this->reg_sym_lit + ")+(( )*,( )*(" + this->reg_sym_lit + ")*)*";
    this->reg_register = "( )*(r([0-7]|sp|pc)|psw)( )*";
    this->reg_register_ind = "( )*(\\[(r([0-7]|sp|pc)|psw)\\])( )*";
    this->reg_reg_plus_lit = "( )*(\\[" + this->reg_register + "\\+( )*(" + this->reg_lit + ")( )*\\])( )*";
    this->reg_reg_plus_sym = "( )*(\\[" + this->reg_register + "\\+( )*(" + this->reg_sym + ")( )*\\])( )*";
    this->reg_reg_lit_dollar = "\\$((-?[0-9]+)|((0x([0-9A-F])+)+))"; // $lit
    this->reg_reg_sym_dollar = "\\$([a-zA-Z]([_a-zA-z0-9])*)"; // $sym
    this->reg_reg_lit_ptr = "(\\*(0x([0-9A-F])+)+)|(\\*[0-9]+)"; // *lit
    this->reg_reg_sym_ptr = "\\*([a-zA-Z]([_a-zA-z0-9])*)"; // *sym
    this->reg_reg_sym_percent = "\\%([a-zA-Z]([_a-zA-z0-9])*)"; // %sym

    // auxiliary patterns for jumps
    this->reg_operand_jmp_label_or_literal = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+((" + this->reg_lit + ")|(" + this->reg_sym + "))( )*((( )|(\t))*#.*)*$"; // jmp literal | jmp sym
    this->reg_operand_jmp_pc_rel = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+(%" + this->reg_sym + ")( )*((( )|(\t))*#.*)*$"; // jmp %sym
    this->reg_operand_jmp_ptr_reg = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+((\\*" + this->reg_register + ")|(\\*" + this->reg_register_ind + "))( )*((( )|(\t))*#.*)*$"; // jmp *reg | jmp *[reg]
    // you have to check if this expression is jmp *reg, because of symbol
    this->reg_operand_jmp_ptr_lit_or_sym = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+((\\*(" + this->reg_lit + "))|(\\*" + this->reg_sym + "))( )*((( )|(\t))*#.*)*$"; // jmp *literal | jmp *sym
    this->reg_operand_jmp_ptr_with_add = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+(\\*(" + this->reg_reg_plus_lit +"|"+ this->reg_reg_plus_sym + "))( )*((( )|(\t))*#.*)*$"; // jmp *[reg+literal] | jmp *[reg+sym]
    this->reg_operand_data = "(" + this->reg_lit + "|" + this->reg_sym + "|" + "%"+this->reg_sym+"|"+"&"+this->reg_lit+"|"+"&"+this->reg_sym+"|"+this->reg_register+"|"+"["+this->reg_register+"]|["+this->reg_register+" + "+this->reg_lit+"]|["+this->reg_register+" + "+this->reg_sym+"])((( )|(\t))*#.*)*";
    this->reg_operand_jmp_reg_plus_lit = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+(\\*(" + this->reg_reg_plus_lit + "))( )*((( )|(\t))*#.*)*$"; // jmp *[reg+lit]
    this->reg_operand_jmp_reg_plus_sym = "^(( )|(\t))*(jmp|jeq|jne|jgt)( )+(\\*(" + this->reg_reg_plus_sym + "))( )*((( )|(\t))*#.*)*$"; // jmp *[reg+sym]

    // auxiliary patterns for .global, halt, iret, ret, 1 register instructions and 2 register instructions
    this->reg_dir_global_str = "(( )|(\t))*.global " + reg_sym_list + "((( )|(\t))*#.*)*$";
    this->reg_inst_halt = "^(( )|(\t))*(halt)( )*((( )|(\t))*#.*)*$";
    this->reg_inst_iret = "^(( )|(\t))*(iret)( )*((( )|(\t))*#.*)*$";
    this->reg_inst_ret = "^(( )|(\t))*(ret)( )*((( )|(\t))*#.*)*$";
    this->reg_inst_one_registar = "^(( )|(\t))*(int|push|pop|not)( )+(r([0-7]|sp|pc)|psw)( )*((( )|(\t))*#.*)*$";
    this->reg_inst_two_registar = "^(( )|(\t))*(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)( )+(r([0-7]|sp|pc)|psw)( )*,( )+(r([0-7]|sp|pc)|psw)( )*((( )|(\t))*#.*)*$";
    
    // auxiliary patterns for call
    // this->reg_operand_call_lit_dollar = "^( )*(call)( )+(" + this->reg_reg_lit_dollar + ")( )*$"; // call $lit
    // this->reg_operand_call_sym_dollar = "^( )*(call)( )+(" + this->reg_reg_sym_dollar + ")( )*$"; // call $sym
    this->reg_operand_call_lit = "^(( )|(\t))*(call)( )+(" + this->reg_lit + ")( )*((( )|(\t))*#.*)*$"; // call lit
    this->reg_operand_call_sym = "^(( )|(\t))*(call)( )+(" + this->reg_sym + ")( )*((( )|(\t))*#.*)*$"; // call sym
    this->reg_operand_call_lit_ptr = "^(( )|(\t))*(call)( )+(" + this->reg_reg_lit_ptr + ")( )*((( )|(\t))*#.*)*$"; // call *lit
    this->reg_operand_call_sym_ptr= "^(( )|(\t))*(call)( )+(" + this->reg_reg_sym_ptr + ")( )*((( )|(\t))*#.*)*$"; // call *sym
    this->reg_operand_call_sym_percent = "^(( )|(\t))*(call)( )+(" + this->reg_reg_sym_percent + ")( )*((( )|(\t))*#.*)*$"; // call %sym
    this->reg_operand_call_reg = "^(( )|(\t))*(call)( )+((\\*" + this->reg_register + "))( )*((( )|(\t))*#.*)*$"; // call *reg
    this->reg_operand_call_reg_mem = "^(( )|(\t))*(call)( )+((\\*" + this->reg_register_ind + "))( )*((( )|(\t))*#.*)*$"; // call *[reg]
    this->reg_operand_call_reg_plus_lit = "^(( )|(\t))*(call)( )+(\\*(" + this->reg_reg_plus_lit + "))( )*((( )|(\t))*#.*)*$"; // call *[reg+lit]
    this->reg_operand_call_reg_plus_sym = "^(( )|(\t))*(call)( )+(\\*(" + this->reg_reg_plus_sym + "))( )*((( )|(\t))*#.*)*$"; // call *[reg+sym]
    
    // auxiliary patterns for ldr and str
    this->reg_operand_ldr_str_lit_dollar = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", " + this->reg_reg_lit_dollar + ")( )*((( )|(\t))*#.*)*$"; // ldr reg, $lit
    this->reg_operand_ldr_str_sym_dollar = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", " + this->reg_reg_sym_dollar + ")( )*((( )|(\t))*#.*)*$"; // ldr reg, $sym
    this->reg_operand_ldr_str_lit = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_lit + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, lit
    this->reg_operand_ldr_str_sym = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_sym + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, sym
    this->reg_operand_ldr_str_sym_percent = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_reg_sym_percent + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, %sym
    this->reg_operand_ldr_str_reg = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_register + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, reg
    this->reg_operand_ldr_str_reg_mem = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_register_ind + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, [reg]]
    this->reg_operand_ldr_str_reg_plus_lit = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_reg_plus_lit + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, [reg+lit]
    this->reg_operand_ldr_str_reg_plus_sym = "^(( )|(\t))*(ldr|str)( )+(" + this->reg_register + ", (" + this->reg_reg_plus_sym + "))( )*((( )|(\t))*#.*)*$"; // ldr reg, [reg+sym]

    // auxiliary basic regex
    this->reg_label_itself = regex("^(( )|(\t))*(" + reg_sym + "):$");
    this->reg_label_and_sth = regex("^(( )|(\t))*(" + reg_sym + "):(.+)$");

    // auxiliary regex for directives
    this->reg_dir_global = regex("^(( )|(\t))*.global " + reg_sym_list + "((( )|(\t))*#.*)*$");
    this->reg_dir_extern = regex("^(( )|(\t))*.extern " + reg_sym_list + "((( )|(\t))*#.*)*$");
    this->reg_dir_section = regex("^(( )|(\t))*.section (" + reg_sym + ")+((( )|(\t))*#.*)*$");
    this->reg_dir_word = regex("^(( )|(\t))*.word (" + reg_sym_lit_list + ")+((( )|(\t))*#.*)*$");
    this->reg_dir_skip = regex("^(( )|(\t))*.skip (" + reg_lit + ")+((( )|(\t))*#.*)*$");
    this->reg_dir_equ = regex("^(( )|(\t))*.equ (" + reg_sym + "), (" + reg_lit + ")((( )|(\t))*#.*)*$");
    this->reg_dir_ascii = regex("^(( )|(\t))*.ascii \"(" + reg_ascii + ")+\"((( )|(\t))*#.*)*");
    this->reg_dir_end = regex("^(( )|(\t))*.end((( )|(\t))*#.*)*$");

    //auxiliary base regex 
    this->regex_comment = regex("(( )|(\t))*#.*");
    this->regex_symbol = regex(reg_sym);
    this->regex_literal = regex(reg_lit);
    this->regex_literal_dec = regex(reg_num);
    this->regex_literal_hex = regex(reg_hex);
    string s1 = "^-?[0-9]+$";
    string s2 = "^(0x([0-9A-F])+)+$";
    this->regex_literal_decONLY = regex(s1);
    this->regex_literal_hexONLY = regex(s2);

    // auxiliary regex for instructions
    this->reg_ins_halt = regex(reg_inst_halt);
    this->reg_ins_iret = regex(reg_inst_iret);
    this->reg_ins_ret = regex(reg_inst_ret);
    this->reg_ins_one_reg = regex(reg_inst_one_registar);
    this->reg_ins_two_reg = regex(reg_inst_two_registar);
    this->reg_ins_call_lit_ptr = regex(reg_operand_call_lit_ptr);
    this->reg_ins_call_sym_ptr = regex(reg_operand_call_sym_ptr);
    this->reg_ins_call_lit = regex(reg_operand_call_lit);
    this->reg_ins_call_sym = regex(reg_operand_call_sym);
    this->reg_ins_call_sym_percent = regex(reg_operand_call_sym_percent);
    this->reg_ins_call_reg = regex(reg_operand_call_reg);
    this->reg_ins_call_reg_mem = regex(reg_operand_call_reg_mem);
    this->reg_ins_call_reg_plus_lit = regex(reg_operand_call_reg_plus_lit);
    this->reg_ins_call_reg_plus_sym = regex(reg_operand_call_reg_plus_sym);
    this->reg_ins_jmp_lab_or_lit = regex(reg_operand_jmp_label_or_literal); 
    this->reg_ins_jmp_pc_rel = regex(reg_operand_jmp_pc_rel);
    this->reg_ins_jmp_ptr_reg = regex(reg_operand_jmp_ptr_reg);
    this->reg_ins_jmp_ptr_lit_or_sym = regex(reg_operand_jmp_ptr_lit_or_sym);
    this->reg_ins_jmp_ptr_with_add = regex(reg_operand_jmp_ptr_with_add);
    this->reg_ins_ldr_str_lit_dollar = regex(reg_operand_ldr_str_lit_dollar);
    this->reg_ins_ldr_str_sym_dollar = regex(reg_operand_ldr_str_sym_dollar);
    this->reg_ins_ldr_str_lit = regex(reg_operand_ldr_str_lit);
    this->reg_ins_ldr_str_sym = regex(reg_operand_ldr_str_sym);
    this->reg_ins_ldr_str_sym_percent = regex(reg_operand_ldr_str_sym_percent);
    this->reg_ins_ldr_str_reg = regex(reg_operand_ldr_str_reg);
    this->reg_ins_ldr_str_reg_mem = regex(reg_operand_ldr_str_reg_mem);
    this->reg_ins_ldr_str_reg_plus_lit = regex(reg_operand_ldr_str_reg_plus_lit);
    this->reg_ins_ldr_str_reg_plus_sym = regex(reg_operand_ldr_str_reg_plus_sym);

    #pragma endregion

    this->input_file = new std::ifstream(file_in, std::ifstream::in);
    this->out_file = file_out;
    this->out1 = std::ofstream("myHelpFileAssembler.txt");
    this->error_file = std::ofstream("errorsAssembler.txt");
    
    out1<<"NOTES";
    out1<<endl;
    out1<<endl;

    this->section = nullptr;
    this->curr_section_name = NO_SECTION;
    this->curr_line = "";
    this->num_curr_line = 0;
    this->error=0;
    this->error_text=ERR_NONE;
    this->location_cnt = 0;
    this->end_single_assembling_process = 0;
    this->cnt = 0;

    this->list_symbols = list<Symbol*>();
    this->list_sections = list<Section*>();

    Section* s = new Section(ABS_SECTION);
    this->list_sections.push_back(s);
    list_symbols.push_back(new Symbol(ABS_SECTION,0,0,SimType_SECTION,ABS_SECTION,0,0,0));

    Section* a = new Section(NO_SECTION);
    this->list_sections.push_back(a);
    list_symbols.push_back(new Symbol(NO_SECTION,0,0,SimType_SECTION,NO_SECTION,0,0,0));

}


void Assembler::single_pass_assembler()
{
    if(input_file->is_open()){
        while(input_file->eof()==0)
        {
            getline(*input_file, curr_line);
            num_curr_line++;

            this->out1 << endl;
            this->out1 <<"line "<< num_curr_line << " read " << "'"+curr_line+"'" << endl;

            curr_line = remove_comment(curr_line);

            //is this an empty line?
            if(curr_line.empty())
            {
                if(input_file->eof())
                {
                    this->out1 << "EOF at line " << curr_line <<endl;
                    //this->out1.close();
                }
                else
                    continue;
            }

            //is this a comment?
            if(is_comment(curr_line)==1)
            {
                this->out1<<"line "<< num_curr_line << " is comment" << " ("+curr_line+")" <<  endl;
                continue;
            }

            //is this a label only?
            if(is_label_only(curr_line)==1)
            {
                this->out1<<"line "<< num_curr_line <<  " is only label (" + curr_line + ")" <<  endl;
                process_label(get_name_of_label(curr_line));
                continue;
            }

            if(is_label_with_sth(curr_line)==1)
            {


                //we need to add this label in symbol table
                string after_label=get_name_after_label(curr_line);
                process_label(get_name_of_label(curr_line));

                this->out1 << "line " << num_curr_line <<  " is label with a command (" + curr_line + ")" << " command: #" << get_name_after_label(curr_line) << "#" << endl;

                Directive dir = is_directive(get_name_after_label(curr_line));
                if( dir != _no_dir )
                {
                    this->out1 << "directive is " + to_string(dir) << " line is: " << after_label << endl;
                    process_directive(dir,1,after_label);
                }
                else{
                    Instruction instr = is_instruction(get_name_after_label(curr_line));
                    //this->out1 << "instruction is " + to_string(dir) << endl;
                    if( instr != NO_INST)
                    {
                        process_instruction(instr,1,after_label);
                    }
                    else{
                        this->error = 1;
                        this->error_text = ERR_NO_DIR_OR_INS;
                        this->out1 << this->error_text << endl;
                    }
                }
            }

            // is this a directive?
            else if(is_directive(curr_line) != _no_dir){
                process_directive(is_directive(curr_line),1,curr_line);
            }

            // is this a instruction?
            else if(is_instruction(curr_line) != NO_INST){
                //this->out1<<"line "<< num_curr_line << " is instr" << " ("+curr_line+")" <<  endl;
                //this->out1 << "inst: " << curr_line << endl;
                //cout << "inst: " << curr_line << endl;
                process_instruction(is_instruction(curr_line),1,curr_line);
            }

        }
        if(input_file->eof())
        {
            backpatching();
            this->out1 << "EOF at line " << num_curr_line <<endl;
            num_curr_line=0;
            write_sym_table();
            generateObjFile(this->out_file);
            this->out1.close();

        }
    }
    else{
        this->out1 << "Input file can't open." << endl;
        cout << "Input file can't open." << endl;
    }
}

string Assembler::remove_comment(string line){
    regex reg = regex(".*#.*");
    smatch m;
    if(regex_match(line,m,reg)){
        int ind = line.find_first_of('#');
        line = line.substr(0,ind);
        line = remove_leading_trailing_spaces(line);
        //this->out1 << "line after removing comment is: *" << line << "*" << endl;
        return line;
    }
    else return line;
}

void Assembler::backpatching() // dovrsi
{
    this->out1 << "U BACKPATCHING-U SMO" << endl;
    for(auto iter = this->list_symbols.begin(); iter != this->list_symbols.end(); iter++)
    {
        Symbol* sym= *iter;
        if(sym->is_part_of_global_dir()==1 && sym->is_defined()==0){
            // check is every .global symbol defined
            this->error=1;
            this->error_text=ERR_UNDEF_GLOB_SYM;
            this->out1<< ERR_UNDEF_GLOB_SYM << endl;
            this->error_file << "ERROR: Symbol: " << sym->get_name() << " is set as global, but it is not defined." << endl;
        }
        else if(sym->is_part_of_extern_dir()==1 && sym->is_defined()==1){
            // check is every .extern symbol defined, if it is defined, that is an error
            this->error = 1;
            this->error_text = ERR_REDEF_EX_SYM;
            this->out1 << this->error_text << endl;
            this->error_file << "ERROR: Symbol: " << sym->get_name() << " is external, but it is redefined." << endl;
        }
        else{
            // if(sym->get_flink() != nullptr && sym->is_defined()==1){
            //     this->out1 << "sredjujem " << sym->get_name() << endl;
            //         for (ST_forwardrefs* it = sym->get_flink(); it!=nullptr; it=it->next)
            //         {
            //             Section* sec = sym->get_section_object();
            //             sec->add_content_at_specific_place(it->size,sym->get_value(),it->loc_cnt);
            //         }
            //}

            if(sym->get_flink() != nullptr){
                this->out1 << "pravim relokacione zapise za simbol: " << sym->get_name() << endl;
                
                    for (ST_forwardrefs* it = sym->get_flink(); it!=nullptr; it=it->next)
                    {
                        cout << it << endl;
                        cout << it->section << endl;
                        if(it->section == nullptr) continue;
                        this->out1 << "section: " << it->section->get_name() << ", offeset: " << it->loc_cnt << ", type: "<<it->type << endl;
                        cout << "section: " << it->section->get_name() << ", offeset: " << it->loc_cnt << ", type: "<<it->type << endl;
                        string section = it->section->get_name();
                        int offset = it->loc_cnt;
                        RelocationRecordType type;
                        string name = "";
                        int addend;
                        if(sym->is_defined()==1 && it->type == R_16 && sym->is_global()==1){
                            this->out1 << "R_16 && global " << endl;
                            type = R_16;
                            name = sym->get_name();
                            int addend = 0;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                            cout << "u asem smo" << endl;
                        }
                        else if(sym->is_defined()==1 && it->type == R_16 && sym->is_global()==0){
                            this->out1 << "R_16 && local " << endl;
                            type = R_16;
                            name = sym->get_section();
                            int s = (Symbol::get_symbol_by_name(name))->get_value(); //vrednost sekcije
                            Symbol* sy = Symbol::get_symbol_by_name(name);
                            int a = sym->get_value();
                            int addend = s + a;
                            this->out1 << "s: " << 1 << ", a: " << a << ", addend: "<< addend << endl;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                        }
                        else if(sym->is_defined()==0 && it->type == R_16){
                            this->out1 << "R_16 && undefined symbol " << endl;
                            type = R_16;
                            name = sym->get_name();
                            int s = Symbol::get_symbol_by_name(name)->get_value(); //vrednost sekcije
                            int a = sym->get_value();
                            int addend = 0;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                        }
                        else if(sym->is_defined()==1 && it->type == R_16PC && sym->is_global()==1){
                            this->out1 << "R_16PC && global " << endl;
                            type = R_16PC;
                            name = sym->get_name();
                            int addend = -2;
                            this->out1 << "addend: "<< addend << endl;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                        }
                        else if(sym->is_defined()==1 && it->type == R_16PC && sym->is_global()==0){
                            this->out1 << "R_16PC && local " << endl;
                            type = R_16PC;
                            name = sym->get_section();
                            int s = sym->get_value(); //vrednost simbola
                            int a = -2;
                            int addend = s + a;
                            this->out1 << "s: " << s << ", a: " << a << ", addend: "<< addend << endl;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                        }
                        else if(sym->is_defined()==0 && it->type == R_16PC){
                            this->out1 << "R_16PC && undefined symbol " << endl;
                            type = R_16PC;
                            name = sym->get_name();
                            int addend = -2;
                            this->out1 << "addend: "<< addend << endl;
                            RelocationRecord* r = new RelocationRecord(section,offset,type,name,addend);
                            it->section->add_relocation_record(*r);
                        }
                    }
            }
        }
    }
}


void Assembler::process_directive(Directive dir, int ser_num, string line)
{
    switch (dir)
    {
    case _global:
        {
            string name = get_name_after_directive(line);

            if( this->list_symbols.size()==0 )
            {
                // if list of all symbols are empty, we need to add this symbol as global
                this->list_symbols.push_back(new Symbol(name,0,0,SimType_DATA,this->curr_section_name,1,0,1));
            }
            else
            {
                int to_add = 0;
                string list_of_params = name;
                std::list<string>list_of_params_to_add;
                if(line.find_first_of(',')!=string::npos){
                    to_add=get_num_of_symbols_in_string_with_commas(list_of_params);
                    this->out1 << "this string(" << list_of_params << ") has " << to_add << " words" <<endl;
                    list_of_params = remove_spaces_from_string(list_of_params);
                    for(int i = 0; i< to_add ; i++){
                        int pos = list_of_params.find_first_of(',');
                        string s = list_of_params.substr(0,pos);
                        this->out1 << s << endl;
                        list_of_params_to_add.push_back(s);
                        list_of_params = list_of_params.substr(pos+1);
                    }
                }
                else{
                    to_add = 1;
                    list_of_params_to_add.push_back(name);
                }
                this->out1 << endl;
                for(int i = 0; i<to_add; i++){
                    // we need to check if there is this symbol already
                    // if it is and it is local, we need to set it as global
                    // if it is and it is global, then it is an error(multiple definition)
                    string n = list_of_params_to_add.front();
                    list_of_params_to_add.pop_front();
                    Symbol* sym = Symbol::get_symbol_by_name(n);
                    if(sym != nullptr){
                        if(sym->is_global()==0)
                            sym->set_global();
                        else
                        {   //error - multiple definition
                            this->error = 1;
                            this->error_text = ERR_MULTIPLE_DEF;
                            this->out1 << this->error_text << endl;
                            this->error_file << ERR_MULTIPLE_DEF << endl;
                            this->error_file << "(at line " << num_curr_line << ")" << endl;
                        };
                    }
                    else{
                        // we need to add this symbol as global and undefined in list of all symbols
                        this->list_symbols.push_back(new Symbol(n,0,0,SimType_DATA,this->curr_section_name,1,0,1));
                    }
                }
            }
        }
        break;
    case _extern:
        {
            
            string list_of_params = get_name_after_directive(line);
            //this->out1 << list_of_params << endl;
            //this->out1 << "list_of_params: " << list_of_params << endl;
            list_of_params = remove_spaces_from_string(list_of_params);
            int to_add = get_num_of_symbols_in_string_with_commas(list_of_params);
            std::list<string> list_of_params_to_add = std::list<string>();
            std::list<string>::iterator it;
            for(int i = 0; i< to_add ; i++){
                int pos = list_of_params.find_first_of(',');
                this->out1 << "list_of_params_to_add[" <<i<< "]=" << list_of_params.substr(0,pos) << endl;
                list_of_params_to_add.push_back(list_of_params.substr(0,pos));
                list_of_params = list_of_params.substr(pos+1);
            }
            if(to_add==0)list_of_params_to_add.push_back(list_of_params);
            
            for(it=list_of_params_to_add.begin();it!=list_of_params_to_add.end();it++){
                string curr = *it;

                // we need to check if there is this symbol already
                // if it is and it is defined, then it is an error
                Symbol* sym = Symbol::get_symbol_by_name(curr);
                
                if(sym != nullptr){
                    if(sym->is_defined()==1){//error - extern symbol 
                        this->error = 1;
                        this->error_text = ERR_REDEF_EX_SYM;
                        this->out1 << this->error_text << endl;
                    }
                }
                else{
                    // we need to add this symbol as global and undefined in list of all symbols
                    Symbol* toAdd = new Symbol(curr,0,0,SimType_DATA,NO_SECTION,1,0,0);
                    toAdd->set_as_part_of_extern_dir();
                    this->list_symbols.push_back(toAdd);
                }
            }
        }
        break;
    case _section:
        {
            string name = get_name_after_directive(line);

            
            // this is the end of previous section
            if(this->section!=nullptr){
                this->section->set_size(this->location_cnt);
            }
            
            this->section=new Section(name);
            this->curr_section_name=name;
            this->list_sections.push_back(this->section);
            this->location_cnt=0;
            this->list_symbols.push_back(new Symbol(name,0,0,SimType_SECTION,this->curr_section_name,0,1,0));

        }
        break;
    case _word: // PROVERITI
        {
            this->out1 << "u word smo" << endl;
            
            
            if(this->section==nullptr){
                this->error=1;
                this->error_text=ERR_NO_SEC;
                this->out1 << ERR_NO_SEC << endl;
                this->out1<<"error"<<endl;
            }
            else{
                string list_of_params=get_name_after_directive(line);
                int to_add=get_num_of_symbols_in_string_with_commas(list_of_params);
                this->out1 << "this string(" << list_of_params << ") has " << to_add << " words" <<endl;

                list_of_params = remove_spaces_from_string(list_of_params);
                std::list<string> list_of_params_to_add=std::list<string>();
                std::list<string>::iterator it;
                if(to_add==0)list_of_params_to_add.push_back(list_of_params);
                for(int i = 0; i< to_add ; i++){
                    int pos = list_of_params.find_first_of(',');
                    list_of_params_to_add.push_back(list_of_params.substr(0,pos));
                    list_of_params = list_of_params.substr(pos+1);
                }
                //this->out1 << endl;
                for(it=list_of_params_to_add.begin();it!=list_of_params_to_add.end();it++){
                    smatch m;
                    long to_insert=0;
                    long higher=0;
                    long lower=0;
                    string curr = *it;
                    if(regex_search(curr,m,regex_literal_hexONLY) |
                    regex_search(curr,m,regex_literal_decONLY)){
                        higher = ((get_numerical_value_from_string(curr))<<8) & 0xFF00;
                        lower = ((get_numerical_value_from_string(curr))>>8) & 0x00FF;
                        to_insert |=higher;
                        to_insert |=lower;
                        this->section->add_content(2,to_insert);
                        this->location_cnt +=2;
                        //this->out1 << "higher: "<< std::hex<< higher<<" lower: "<<std::hex<< lower<<"to_insert: " <<std::hex<< to_insert<<endl;
                    }
                    else if(regex_search(curr,m,regex_symbol)){ //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DOBRSI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        Symbol* sym = Symbol::get_symbol_by_name(curr);
                        string sec = curr_section_name;
                        int offs = this->location_cnt;
                        RelocationRecordType type = R_16;
                        string name_for_table = "";
                        string real_name = "";
                        int addend = 0;

                        this->out1 << "word and sym: " << sym->get_name() << endl;
                        if(sym!=nullptr){ // symbol vec postoji u tabeli simbola
                            if(sym->get_section().compare(ABS_SECTION)!=0){
                                // simbol nije definisan
                                // mozda ce biti posle
                                // dodajemo flink
                                // on postoji u tabeli simbola, samo se doda flink

                                // dodajemo nule u sadrzaj sekcije
                                sym->insert_flink(this->location_cnt,2,this->section,R_16);
                                this->out1 << "flink addend" << endl;

                            }
                            else{
                                lower=sym->get_value();
                                to_insert |=lower;
                                sym->insert_flink(this->location_cnt,2,this->section,R_16);
                                this->out1 << "flink addend" << endl;
                            }
                            this->section->add_content(2,to_insert);
                            this->location_cnt +=2;
                            //this->out1 << "higher: "<< std::hex<< higher<<" lower: "<<std::hex<< lower<<" to_insert: " <<std::hex<< to_insert<<endl;
                        }
                        else{
                            // simbol ne postoji u tabeli simbola
                            // dodaje se, zajedno sa flinkom
                            // a u sadrzaj sekcije upisujemo 0
                            Symbol* sym = new Symbol(curr,0,2,SimType_DATA,"",0,0,0);
                            sym->insert_flink(this->location_cnt,2,this->section,R_16);
                            this->list_symbols.push_back(sym);
                            this->section->add_content(2,to_insert);
                            this->location_cnt+=2;
                        }
                    }
                    else{
                        this->out1 << "it is not literal or symbol" << endl;
                    }
                }
            }
            
        }
        break;
    case _skip:
        {
            
            if(this->section==nullptr){
                this->error=1;
                this->error_text=ERR_NO_SEC;
                this->out1 << ERR_NO_SEC << endl;
            }
            else{
                string name=get_name_after_directive(line);
                int to_add=get_numerical_value_from_string(name);
                for(int i=0; i<to_add;i++){
                this->section->add_content(1,0);
                }   
                this->location_cnt += to_add;
            }
        }
        break;
    case _equ:
        {
            
            // we need to check if there is this symbol already
            // if it is and it is defined, then it is an error
            string name= get_name_from_equ(line);
            int literal = get_numerical_value_from_string(get_literal_from_equ(line));
            //this->out1 << "equ dir; name: " << name << ", value: " << literal << endl;

            Symbol* sym = Symbol::get_symbol_by_name(name);
            if(sym != nullptr){
                if(sym->is_defined()==1){//error - extern symbol 
                    this->error = 1;
                    this->error_text = ERR_MULTIPLE_DEF;
                    this->error_file << ERR_MULTIPLE_DEF << endl;
                    this->error_file << "(at line " << num_curr_line << ")" << endl;
                }
                else{ // we need to add new inf to this symbol
                //std::string name, int value, int size, int type, std::string section, int binding, int is_defined_flag
                    sym->set_value(literal);
                    sym->set_size(get_num_of_bytes_in_integer(literal));
                    sym->set_type(SimType_ABS);
                    sym->set_section(ABS_SECTION);
                    sym->set_as_defined();
                    int size = get_num_of_bytes_in_integer(literal);
                    cout <<"sizeeee isssss " << size << " |||||||||||||||||||||||| literal is " << literal << endl;
                    Section* s = Section::get_section_by_name(ABS_SECTION);
                    short toAdd0 = 0;
                    short toAdd1 = 0;
                    short toAdd2 = 0;
                    short toAdd3 = 0;
                    if(size==1 || size==2){
                        toAdd0 = (short)literal & 0xFF;
                        toAdd1 = (short)(literal>>8) & 0xFF;
                        s->add_content(1,toAdd0);
                        s->add_content(1,toAdd1);
                        s->set_size(s->get_size()+2);
                    }
                    else if(size==3 || size==4){
                        toAdd0 = (short)literal & 0xFF;
                        toAdd1 = (short)(literal>>8) & 0xFF;
                        toAdd2 = (short)(literal>>16) & 0xFF;
                        toAdd3 = (short)(literal>>24) & 0xFF;
                        s->add_content(1,toAdd0);
                        s->add_content(1,toAdd1);
                        s->add_content(1,toAdd2);
                        s->add_content(1,toAdd3);
                        s->set_size(s->get_size()+4);
                    }
                }
            }
            else{
                // we need to add this symbol as local and defined in list of all symbols
                int size = get_num_of_bytes_in_integer(literal);
                //this->out1 << "size of literal in bytes is: " << size << endl;
                Symbol* sym = new Symbol(name,literal,size,SimType_ABS,ABS_SECTION,0,1,0);
                this->list_symbols.push_back(sym);
                cout <<"sizeeee isssss " << size << " |||||||||||||||||||||||| literal is " << literal << endl;
                Section* s = Section::get_section_by_name(ABS_SECTION);
                short toAdd0 = 0;
                short toAdd1 = 0;
                short toAdd2 = 0;
                short toAdd3 = 0;
                if(size==1 || size==2){
                    toAdd0 = (short)literal & 0xFF;
                    toAdd1 = (short)(literal>>8) & 0xFF;
                    s->add_content(1,toAdd0);
                    s->add_content(1,toAdd1);
                    s->set_size(s->get_size()+2);
                }
                else if(size==3 || size==4){
                    toAdd0 = (short)literal & 0xFF;
                    toAdd1 = (short)(literal>>8) & 0xFF;
                    toAdd2 = (short)(literal>>16) & 0xFF;
                    toAdd3 = (short)(literal>>24) & 0xFF;
                    s->add_content(1,toAdd0);
                    s->add_content(1,toAdd1);
                    s->add_content(1,toAdd2);
                    s->add_content(1,toAdd3);
                    s->set_size(s->get_size()+4);
                }
        }
        }
        break;
    case _ascii:
        {
            
            if(this->section==nullptr){
                this->error=1;
                this->error_text=ERR_NO_SEC;
                this->out1 << ERR_NO_SEC << endl;
                this->out1<<"error"<<endl;
            }
            else if(this->section->get_name() == "bss")
            {
                this->error=1;
                this->error_text=ERR_ASCII_PART_OF_BSS;
                this->out1 << ERR_ASCII_PART_OF_BSS << endl;
                this->out1<<"error"<<endl; 
            }
            else{
                string name=get_name_after_directive(line);
                if(name.find_first_of("\"") != string::npos) // we have now format "string", but we want only string
                {
                    this->out1 << "before removing \" from string: " << name << endl;
                    name = name.substr(1, name.length()-2);
                    this->out1 << "after removing \" from string: " << name << endl;
                }
                int to_add=name.length();
                this->location_cnt += to_add;
                this->out1 << "this string(" << name << ") has " << to_add << " words" <<endl;
                for(int i = 0; i<to_add;i++)
                {
                    long to_add_string = get_hex_values_from_char(name[i]);
                    this->section->add_content_big_endian(to_add_string);
                    this->out1 << "we added string as ascii characters in section content as: " << to_add_string << endl;
                }
            }
            
        }
        break;
    case _end:
        {
            
            if(this->section!=nullptr){
                cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;
                this->section->set_size(this->location_cnt);
            }
            this->end_single_assembling_process = 1;
            
        }
        break;
    
    default:
        {
            this->error = 1;
            this->error_text = ERR_UNKNOWN_COMMAND;
            if(ser_num==1){
                this->out1 << this->error_text << endl;
            }
        }
        break;
    }
}
// InstrDescr RegsDescr AddrMode DataHigh DataLow
//     1         2         3        4       5
// InstrDescr OC3 OC2 OC1 OC0 MOD3 MOD2 MOD1 MOD0 
// RegsDescr RD3 RD2 RD1 RD0 RS3 RS2 RS1 RS0 
// AddrMode Up3 Up2 Up1 Up0 AM3 AM2 AM1 AM0
void Assembler::process_instruction(Instruction instr, int ser_num, string line){
    cout<< "INSTR #"<<to_string(instr)<<"#"<<endl;
    this->out1 << "INSTR #"<<to_string(instr)<<"#"<<endl;
    long to_insert = 0;
    switch (instr)
    {
    case HALT:
        {
            // FORMAT: 0000
            //to_insert |= (this->get_instr_descr(instr) << 12); // zasto pomeram ulevo za 12???
            to_insert |= this->get_instr_descr(instr);
            this->section->add_content(1, to_insert);
            this->location_cnt+=1;
        }
        break;
    case IRET:
    case RET:
        {
            // FORMAT IRET: 0010 0000
            // FORMAT RET: 0100 0000
            to_insert |= this->get_instr_descr(instr);
            this->section->add_content(1, to_insert);
            this->location_cnt+=1;
        }
        break;
    case CALL:
        {
            // InstrDescr RegsDescr AddrMode DataHigh DataLow
            //     1         2         3        4       5
            // InstrDescr OC3 OC2 OC1 OC0 MOD3 MOD2 MOD1 MOD0 
            // RegsDescr RD3 RD2 RD1 RD0 RS3 RS2 RS1 RS0 
            // AddrMode Up3 Up2 Up1 Up0 AM3 AM2 AM1 AM0

            // OC - operacioni kod
            // MOD - modifikator

            // Rd - indeks destination registra
            // Rs - indeks source registra
            //  PC je r7, SP je r6 (poslednja zauzeta, stek raste ka nizim adresama), PSW je r8

            // Up3210 nacin azuriranja kod regind
            // Am3210 nacin adresiranja : 0 neposredno, 1 regdir, 

            int instr_desc = this->get_instr_descr(instr); // 0x30 -> 0011 0000 1.B
            this->section->add_content(1, instr_desc);
            int regD = 15; // regD is 1111


            smatch m;
            if(regex_search(line,m,reg_ins_call_reg))
            {
                //FORMAT call *reg
                cout << "regex_search(line,m,reg_ins_call_reg)" << endl;

                int addr_mode = 0x01;
                int regS = this->get_num_of_reg_jmps(line);
                to_insert |= (regD << 12) | (regS << 8) | addr_mode;
                this->section->add_content(2, to_insert);
                this->location_cnt+=3;
            }
            else if(regex_search(line,m,reg_ins_call_reg_mem)){
                //FORMAT call *[reg]
                cout << "regex_search(line,m,reg_ins_call_reg)" << endl;
                int addr_mode = 0x02;
                int regS = this->get_num_of_reg_jmps(line);
                to_insert |= (regD << 12) | (regS << 8) | addr_mode;
                this->section->add_content(2, to_insert);
                this->location_cnt+=3;
            }
            else if(regex_search(line,m,reg_ins_call_lit) | regex_search(line,m,reg_ins_call_lit_ptr))
            {
                // call literal | call *literal
                // DataHigh and DataLow are literal from instruction
                cout << "regex_search(line,m,reg_ins_call_lit) | regex_search(line,m,reg_ins_call_lit_ptr) " << endl;
                int addr_mode = 0x00; //immediate
                string op = get_string_after_instruction(line);
                if(op.find_first_of("*") !=  string::npos) {
                    op = op.substr(1);
                    addr_mode = 0x04; // memory addressing
                }
                int opInt = get_numerical_value_from_string(op);
                int firstToAdd = opInt & 0xFF;
                int secondToAdd = (opInt>>8) & 0xFF;
                int regS = 0;
                to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | (firstToAdd << 8) | secondToAdd;
                this->section->add_content(4, to_insert);
                this->location_cnt+=5;
            } 
            else if(regex_search(line,m,reg_ins_call_reg_plus_lit) )
            {
                //FORMAT call *[reg + lit]
                cout << "regex_search(line,m,reg_ins_call_reg_plus_lit)  " << endl;
                int addr_mode = 0x03;
                int regS = this->get_num_of_reg_jmps(line);
                string litStr = get_lit_ot_sym_from_jmps_with_add(line);
                int val = get_numerical_value_from_string(litStr);
                int firstToAdd = val & 0xFF;
                int secondToAdd = (val>>8) & 0xFF;
                to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | (firstToAdd << 8) | secondToAdd;
                this->section->add_content(4, to_insert);
                this->location_cnt+=5;
            }
            else if(regex_search(line,m,reg_ins_call_reg_plus_sym))
            {
                //FORMAT call *[reg + lit]
                cout << "regex_search(line,m,reg_ins_call_reg_plus_sym)" << endl;
                int addr_mode = 0x03;
                int regS = this->get_num_of_reg_jmps(line);
                string sym_name = get_lit_ot_sym_from_jmps_with_add(line);

                Symbol* sym = Symbol::get_symbol_by_name(sym_name);
                if(sym != nullptr){
                    // there is this symbol
                    if(sym->is_defined()){
                        // definisan simbol, imamo vrednost
                        // ugradi vrednost
                        int value = sym->get_value();
                        int firstToAdd = value & 0xFF;
                        int secondToAdd = (value>>8) & 0xFF;
                        to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | (firstToAdd << 8) | secondToAdd;
                        this->section->add_content(4, to_insert);

                        //ALI TREBA NAM FLINK
                        //JER CE SIMBOL MOZDA IMATI DRUGU VREDOST KAD SE ISPOMERA
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                    else{
                        // symbol is not defined, we have to add FLINK
                        int value = 0;
                        to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
                        this->section->add_content(4, to_insert);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                }
                else{
                    // there is not this symbol
                    /*
                    Ako labela nije nađena u tabeli simbola, 
                    biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                    Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                    U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                    tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                    */
                   Symbol* sym = new Symbol(sym_name,0,2,SimType_DATA,this->curr_section_name,0,0,0);
                   sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                   this->list_symbols.push_back(sym);
                }

                this->location_cnt+=5;
                
            }
            else if(regex_search(line,m,reg_ins_call_sym) | regex_search(line,m,reg_ins_call_sym_ptr))
            {
                // call sym | call *sym

                cout << "regex_search(line,m,reg_ins_call_sym) | regex_search(line,m,reg_ins_call_sym_ptr)" << endl;
                int addr_mode = 0x00; //immediate
                string name = get_string_after_instruction(line);
                if(name.find_first_of("*") !=  string::npos) {
                    name = name.substr(1);
                    addr_mode = 0x04; // memory addressing
                }
                int regS = 0;

                Symbol* sym = Symbol::get_symbol_by_name(name);
                if(sym != nullptr){
                    // there is this symbol
                    if(sym->is_defined()){
                        // definisan simbol, imamo vrednost
                        // ugradi vrednost
                        int value = sym->get_value();
                        int firstToAdd = value & 0xFF;
                        int secondToAdd = (value>>8) & 0xFF;
                        to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | (firstToAdd << 8) | secondToAdd;
                        this->section->add_content(4, to_insert);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                    else{
                        // symbol is not defined, we have to add FLINK
                        int value = 0;
                        to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
                        this->section->add_content(4, to_insert);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                }
                else{
                    // there is not this symbol
                    /*
                    Ako labela nije nađena u tabeli simbola, 
                    biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                    Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                    U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                    tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                    */
                   Symbol* sym = new Symbol(name,0,2,SimType_DATA,this->curr_section_name,0,0,0);
                   sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                   this->list_symbols.push_back(sym);
                }
                this->location_cnt+=5;

            }
            else if(regex_search(line,m,reg_ins_call_sym_percent))
            {
                // call %sym
                this->out1 << "regex_search(line,m,reg_ins_call_sym_percent)" << endl;
                string op = get_string_after_instruction(line);
                int i = op.find_first_of("%");
                op = op.substr(i+1, op.size()-1);
                int addr_mode = 0x00;
                int regS = 7;
                to_insert |= (regD << 12) | (regS << 8) | (addr_mode );
                this->section->add_content(2, to_insert);


                
                Symbol* sym = Symbol::get_symbol_by_name(op);
                if(sym != nullptr){
                    // there is this symbol
                    if(sym->is_defined()){
                        // definisan simbol, imamo vrednost
                        // ugradi vrednost
                        int value = sym->get_value();
                        int firstToAdd = value & 0xFF;
                        int secondToAdd = (value>>8) & 0xFF;
                        value = (firstToAdd << 8) | secondToAdd;
                        this->section->add_content(2, value);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                    else{
                        // symbol is not defined, we have to add FLINK
                        int value = 0;
                        this->section->add_content(2, value);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                    }
                }
                else{
                    // there is not this symbol
                    /*
                    Ako labela nije nađena u tabeli simbola, 
                    biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                    Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                    U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                    tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                    */
                   Symbol* sym = new Symbol(op,0,2,SimType_DATA,this->curr_section_name,0,0,0);
                   sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                   this->list_symbols.push_back(sym);
                }            

                this->location_cnt+=5;

            }
            

            cout << "                                    loc_cnt jeeee (hex) " << std::dec << this->location_cnt << endl;
        }
        break;
    case JMP:
    case JEQ:
    case JNE:
    case JGT:
        {
            this->out1 << "WE ARE PROCESSING JUMPS" << endl;
            int instr_desc = this->get_instr_descr(instr); // 0x5? -> 0011 0000 1.B
            this->section->add_content(1, instr_desc);
            int regD = 15; // regD is 1111
            int addr_mode = 0;
            int regS = 0;


            smatch m;
            if(regex_search(line,m,reg_ins_jmp_ptr_reg)){
                // jmp *reg | jmp *[reg]
                if(line.find_first_of("[") != string::npos){
                    // jmp *[reg]
                    cout << "regex_search(line,m,reg_ins_jmp_ptr_reg) - jmp *[reg]" << endl;

                    addr_mode = 0x02;
                    regS = this->get_num_of_reg_jmps(line);
                }
                else{
                    // jmp *reg
                    cout << "regex_search(line,m,reg_ins_jmp_ptr_reg) - jmp *reg" << endl;

                    addr_mode = 0x01;
                    regS = this->get_num_of_reg_jmps(line);
                }
                to_insert |= (regD << 12) | (regS << 8) | addr_mode;
                this->section->add_content(2, to_insert);
                this->location_cnt+=3;
                cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;

            }
            else if(regex_search(line,m,reg_ins_jmp_lab_or_lit) | regex_search(line,m,reg_ins_jmp_ptr_lit_or_sym )){
                 // jmp literal | jmp *literal | jmp sym | jmp *sym
                if(line.find_first_of("*") != string::npos) addr_mode = 0x04;
                to_insert |= (regD << 12) | (regS << 8) | addr_mode;
                this->section->add_content(2, to_insert);

                if(regex_search(line,m,regex_literal_decONLY) | regex_search(line,m,regex_literal_hexONLY)){
                    // jmp literal 
                    cout << "regex_search(line,m,regex_literal_dec) | regex_search(line,m,regex_literal_hex)- jmp literal " << endl;
                    
                    string op = get_string_after_instruction(line);
                    if(op.find_first_of("*") != string::npos) op = op.substr(1);
                    int opInt = get_numerical_value_from_string(op);
                    int firstToAdd = opInt & 0xFF;
                    int secondToAdd = (opInt>>8) & 0xFF;
                    opInt = (firstToAdd << 8) | secondToAdd;
                    this->section->add_content(2, opInt);

                }
                else{
                    // jmp sym
                    cout << "jmp sym | jmp *sym " << endl;

                    string name = get_string_after_instruction(line);
                    this->out1 << "symbol after instr is: " << name << endl;
                    if(name.find_first_of("*") != string::npos) name = name.substr(1);

                    Symbol* sym = Symbol::get_symbol_by_name(name);
                    if(sym != nullptr){
                        // there is this symbol
                        if(sym->is_defined()){
                            // definisan simbol, imamo vrednost
                            // ugradi vrednost
                            int value = sym->get_value();
                            int firstToAdd = value & 0xFF;
                            int secondToAdd = (value>>8) & 0xFF;
                            value = (firstToAdd << 8) | secondToAdd;
                            this->out1 << "symbol is already defined, we add his value" << endl;
                            this->section->add_content(2, value);

                            //ALI TREBA NAM FLINK
                            //JER CE SIMBOL MOZDA IMATI DRUGU VREDOST KAD SE ISPOMERA
                            sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        }
                        else{
                            // symbol is not defined, we have to add FLINK
                            int value = 0;
                            this->out1 << "symbol is already defined, we add 0" << endl;
                            this->section->add_content(2, value);
                            sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        }
                    }
                    else{
                        // there is not this symbol
                        /*
                        Ako labela nije nađena u tabeli simbola, 
                        biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                        Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                        U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                        tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                        */
                        Symbol* sym = new Symbol(name,0,2,SimType_DATA,"",0,0,0);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        this->section->add_content(2, 0);
                        this->list_symbols.push_back(sym);
                    }
                }
                
                this->location_cnt+=5;
                cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;
            } 
            else if(regex_search(line,m,reg_ins_jmp_pc_rel)){
                // jmp %sym
                cout << "regex_search(line,m,reg_ins_jmp_pc_rel) - jmp %sym" << endl;
                string op = get_string_after_instruction(line);
                int i = op.find_first_of("%");
                op = op.substr(i+1, op.size()-1);
                int addr_mode = 0x05; // regS is PC (r7) + operand, regdir with add
                int regS = 7;
                to_insert |= (regD << 12) | (regS << 8) | (addr_mode );
                this->section->add_content(2, to_insert);
                
                Symbol* sym = Symbol::get_symbol_by_name(op);
                if(sym != nullptr){
                    // there is this symbol
                    if(sym->is_defined()){
                        // definisan simbol, imamo vrednost
                        // ugradi vrednost
                        int value = sym->get_value();
                        int firstToAdd = value & 0xFF;
                        int secondToAdd = (value>>8) & 0xFF;
                        value = (firstToAdd << 8) | secondToAdd;
                        this->section->add_content(2, value);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16PC);
                    }
                    else{
                        // symbol is not defined, we have to add FLINK
                        int value = 0;
                        this->section->add_content(2, value);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16PC);
                    }
                }
                else{
                    // there is not this symbol
                    /*
                    Ako labela nije nađena u tabeli simbola, 
                    biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                    Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                    U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                    tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                    */
                   Symbol* sym = new Symbol(op,0,2,SimType_DATA,this->curr_section_name,0,0,0);
                   sym->insert_flink(this->location_cnt+3,2,this->section,R_16PC);
                   this->list_symbols.push_back(sym);
                }            

                this->location_cnt+=5;
                cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;
            }
            else if(regex_search(line,m,reg_ins_jmp_ptr_with_add)){

                if(regex_search(line,m,reg_operand_jmp_reg_plus_lit)){
                    // *[reg + lit]
                    cout << "*[reg + lit]  " << endl;
                    int addr_mode = 0x03;
                    int regS = this->get_num_of_reg_jmps(line);
                    string litStr = get_lit_ot_sym_from_jmps_with_add(line);
                    int val = get_numerical_value_from_string(litStr);
                    int firstToAdd = val & 0xFF;
                    int secondToAdd = (val>>8) & 0xFF;
                    val = (firstToAdd << 8) | secondToAdd;
                    to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | val;
                    this->section->add_content(4, to_insert);
                }
                else{
                    // *[reg + sym]
                    cout << "regex_search(line,m,reg_ins_call_reg_plus_sym)" << endl;
                    int addr_mode = 0x03;
                    int regS = this->get_num_of_reg_jmps(line);
                    cout << " IME CEMOO SAZNATI: " << endl;
                    string sym_name = get_lit_ot_sym_from_jmps_with_add(line);
                    cout << sym_name + " :)" << endl;

                    Symbol* sym = Symbol::get_symbol_by_name(sym_name);
                    if(sym != nullptr){
                        // there is this symbol
                        if(sym->is_defined()){
                            // definisan simbol, imamo vrednost
                            // ugradi vrednost
                            int value = sym->get_value();
                            int firstToAdd = value & 0xFF;
                            int secondToAdd = (value>>8) & 0xFF;
                            value = (firstToAdd << 8) | secondToAdd;
                            to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | value;
                            this->section->add_content(4, to_insert);
                            sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        }
                        else{
                            // symbol is not defined, we have to add FLINK
                            int value = 0;
                            to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
                            this->section->add_content(4, to_insert);
                            sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        }
                    }
                    else{
                        // there is not this symbol
                        /*
                        Ako labela nije nađena u tabeli simbola, 
                        biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                        Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                        U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                        tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                        */
                        Symbol* sym = new Symbol(sym_name,0,2,SimType_DATA,this->curr_section_name,0,0,0);
                        sym->insert_flink(this->location_cnt+3,2,this->section,R_16);
                        this->list_symbols.push_back(sym);
                    }
                    
                }
                this->location_cnt+=5;
                cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;
            }
        }
        break;
    case PUSH:
    case POP:
        {
            //FORMAT POP 1010 0000 regD regS 0100 0010
            //FORMAT PUSH 1011 0000 regD regS 0001 0010
            this->out1 << "THIS IS PUSH INSTRUCTION" << endl;
            int instr_desc = this->get_instr_descr(instr);
            int regD = this->get_num_of_reg_one_reg_inst(line);
            int regS = 6; // r6 je SP
            int addr_mode = (instr==PUSH)? 0x12 : 0x42; 
            to_insert |= (instr_desc << 16) | (regD << 12) | (regS << 8) | addr_mode;
            this->section->add_content(3, to_insert);
            this->location_cnt+=3;
        }
        break;
    case INT:
        {
            //FORMAT 0001 0000 regD 1111
            int regD = this->get_num_of_reg_one_reg_inst(line);
            to_insert |=(1 << 12) | (regD << 4) | 15;
            this->section->add_content(2,to_insert);
            this->location_cnt+=2;
            
        }
        break;
    case XCHG:
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case CMP:
    case NOT:
    case AND:
    case OR:
    case XOR:
    case TEST:
    case SHL:
    case SHR:
        {
            //FORMAT OpOpOpOp MMMM regD regS
            this->out1 << "processing instruction " << to_string(instr) << endl;
            if(instr==XCHG) to_insert |= (this->get_instr_descr(XCHG) << 12) | (0 << 8);
            else to_insert |= (this->get_instr_descr(instr) << 8);
            int regD = this->get_num_of_first_reg_two_reg_inst(line);
            int regS = 0;
            if(instr != NOT) regS = this->get_num_of_second_reg_two_reg_inst(line);
            to_insert |= (regD << 4) | regS;
            this->section->add_content(2,to_insert);
            this->location_cnt+=2;
        }
        break;
    case LDR:
    case STR:
        {
            process_ldr_str_instruction(line,instr);
        }
        break;
    
    default:
        break;
    }
}

void Assembler::process_label(string label){
    this->out1 << "LABEL IS " << label << endl;

    if(this->curr_section_name==NO_SECTION){
        this->error=1;
        this->error_text=ERR_NO_SEC;
        this->out1<< "ERROR - label is not a part of any section" << endl;
        cout<< "ERROR - label is not a part of any section" << endl;
    }
    else{
        // we need to check if this label is already defined
        for(auto iter = this->list_symbols.begin(); iter != this->list_symbols.end(); iter++)
        {
            Symbol* sym= *iter;
            this->out1 << "symbol: " << sym->get_name() << " is in section: " << sym->get_section() << ", curr section is: " << curr_section_name << endl;
            if(sym->get_section().compare("")!=0){ 
                // postoji u tabeli simbola
                // i u istoj je sekciji
                if(sym->get_name().compare(label)==0 && sym->get_section().compare(this->curr_section_name)==0 && sym->is_defined()==1){
                this->error=1;
                this->error_text=ERR_DEF_LAB_IN_SEC;
                this->out1<< "ERROR - label has already defined in this section" << endl;
                this->out1 << "name: " << sym->get_name() << " ,section: " << sym->get_section() << " ,is_defined: " << sym->is_defined() << " ,curr section: " << curr_section_name << endl;
                cout<< "ERROR - label has already defined in this section" << endl;
                return;
                }
                
            }
            if(sym->get_name().compare(label)==0 && sym->is_defined()==0){
                // postoji u tabeli, ali nije definisan
                // dodefinisemo ga
                sym->set_as_defined();
                sym->set_section(this->curr_section_name);
                sym->set_section_object(this->section);
                sym->set_value(this->location_cnt);
            }
        }
        if(Symbol::get_symbol_by_name(label)==nullptr){
            this->out1 << "Label: " << label << " added in symbol table. (section: " << curr_section_name << ", defined: 1)" <<endl;
            Symbol* sym = new Symbol(label,0,0,SimType_DATA,"",0,0,0);
            sym->set_as_defined();
            sym->set_section(this->curr_section_name);
            sym->set_section_object(this->section);
            sym->set_value(this->location_cnt);
            this->list_symbols.push_back(sym);
        }
    }
}

void Assembler::process_ldr_str_instruction(string line, Instruction instr){
    // LDR: 1010 0000 DDDD SSSS UUUU AAAA
    // STR: 1011 0000 DDDD SSSS UUUU AAAA
    cout << "elmir" << endl;
    this->out1 << "elmir" << endl;
    int instr_desc = this->get_instr_descr(instr);
    this->section->add_content(1, instr_desc);

    long to_insert = 0;
    int regD = this->get_num_of_regD_ldr_str(line);
    this->out1 << "regD in ldr/str regD is: " << regD << endl;
    smatch m;

    if(regex_search(line,m,reg_ins_ldr_str_reg)){
        // LDR: 1010 0000 DDDD SSSS 0000 0001
        // STR: 1011 0000 DDDD SSSS 0000 0001
        // ldr/str regD, regS
        this->location_cnt+=3;
        string oprndString = get_operand_ldr_str(line);
        cout << "oprndString in ldr/str regD, regS is #" << oprndString << "#" << endl;
        string s = "push " + oprndString;
        int oprnd = (oprndString != "6" && oprndString != "7" && oprndString != "8") ? get_num_of_reg_one_reg_inst(s) : get_numerical_value_from_string(oprndString);
        to_insert |= (regD << 12) | (oprnd << 8);
        to_insert |= 0x1;
        this->section->add_content(2,to_insert);
        this->out1 << "regD, regS: " << regD << " " << oprnd <<endl;
        this->out1 << "Ldr/str regD, regX; to_insert should be 1010(1011) 0000 DDDD SSSS 0000 0001, and it is: " << std::hex << to_insert << endl;
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg_mem)){
        // LDR: 1010 0000 DDDD SSSS 0000 0010
        // STR: 1011 0000 DDDD SSSS 0000 0010
        // ldr/str regD, [regS]
        this->location_cnt+=3;
        string oprndString = get_operand_ldr_str(line);
        cout << "oprndString in ldr/str regD, [regS] is #" << oprndString << "#" << endl;
        if(oprndString.find("[") != string::npos) oprndString = oprndString.substr(1, oprndString.size()-2);
        string s = "push " + oprndString;
        int oprnd = (oprndString != "6" && oprndString != "7" && oprndString != "8") ? get_num_of_reg_one_reg_inst(s) : get_numerical_value_from_string(oprndString);
        this->out1 << "Num of second reg in 'ldr/str regD, [regS]' instr is: " << to_string(oprnd) << endl;
        to_insert |= (regD << 12) | (oprnd << 8);
        to_insert |= 0x2;
        this->section->add_content(2,to_insert);
        this->out1 << "regD, [regS]: " << regD << " " << oprnd <<endl;
        this->out1 << "Ldr/str regD, [regX]; to_insert should be 1010(1011) 0000 DDDD SSSS 0000 0010, and it is: " << std::hex << to_insert << endl;
    
    }
    else if( regex_search(line,m,reg_ins_ldr_str_lit)){
        // LDR: 1010 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL
        // STR: 1011 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL
        // ldr/str regD, lit
        this->location_cnt+=5;
        string oprndString = get_operand_ldr_str(line);
        int oprnd = get_numerical_value_from_string(oprndString);
        // for(int i = 0; i<10; i++){
            //cout<< "                                                       m.str(" << i <<"): " << m.str(i) << endl;
        // }
        this->out1 << "Literal in ldr/str regD, lit is: " << oprnd << endl;
        int firstToAdd = oprnd & 0xFF;
        int secondToAdd = (oprnd>>8) & 0xFF;
        oprnd = (firstToAdd << 8) | secondToAdd;

        to_insert |= (regD << 28) | (0xF << 24);
        to_insert |= (0x04 << 16);
        to_insert |= oprnd;
        this->section->add_content(4,to_insert);
        this->out1 << "regD, literal: " << regD << " " << oprnd <<endl;
        this->out1 << "Ldr/str regD, literal; to_insert should be 1010(1011) 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL, and it is: " << std::hex << to_insert << endl;
    
    }
    else if(regex_search(line,m,reg_ins_ldr_str_lit_dollar)){
        // LDR: 1010 0000 DDDD 1111 0000 0000 HHHH HHHH LLLL LLLL
        // STR: 1011 0000 DDDD 1111 0000 0000 HHHH HHHH LLLL LLLL
        // ldr/str regD, $lit
        this->location_cnt+=5;
        smatch m;
        string oprndString = get_operand_ldr_str(line);
        int oprnd = get_numerical_value_from_string(oprndString);
        this->out1 << "Literal in ldr/str regD, $lit is: " << oprnd << endl;
        int firstToAdd = oprnd & 0xFF;
        int secondToAdd = (oprnd>>8) & 0xFF;
        oprnd = (firstToAdd << 8) | secondToAdd;
        to_insert |= (regD << 28) | (0xF << 24);
        to_insert |= (0x00 << 16);
        to_insert |= oprnd;
        this->section->add_content(4,to_insert);
        this->out1 << "regD, $literal: " << regD << " " << oprnd <<endl;
        this->out1 << "Ldr/str regD, literal; to_insert should be 1010(1011) 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL, and it is: " << std::hex << to_insert << endl;
    
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg_plus_lit) |
    regex_search(line,m,reg_ins_ldr_str_reg_plus_sym)){

        if(regex_search(line,m,reg_ins_ldr_str_reg_plus_lit)){
                    // [reg + lit]
                    int addr_mode = 0x03;
                    string oprndString = get_lit_ot_sym_from_jmps_with_add(line);
                    int val = get_numerical_value_from_string(oprndString);
                    int firstToAdd = val & 0xFF;
                    int secondToAdd = (val>>8) & 0xFF;
                    val = (firstToAdd << 8) | secondToAdd;
                    int regS = get_num_of_reg_jmps(line);
                    this->out1 << "regD, [regS + lit]: " << regD << " " << regS << " " << val <<endl;
                    to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | val;
                    this->section->add_content(4, to_insert);
        }
        else{
            // *[reg + sym]
            cout << "regex_search(line,m,reg_ins_ldr_str_reg_plus_sym)" << endl;
            int addr_mode = 0x03;
            string sym_name = get_lit_ot_sym_from_jmps_with_add(line);
            int regS = get_num_of_reg_jmps(line);
            cout << sym_name + " :)" << endl;
            this->out1 << "regD, [regS + sym]: " << regD << " " << regS << " " << sym_name <<endl;

            Symbol* sym = Symbol::get_symbol_by_name(sym_name);
            if(sym != nullptr){
                // there is this symbol
                if(sym->is_defined()){
                    // definisan simbol, imamo vrednost
                    // ugradi vrednost
                    int value = sym->get_value();
                    int firstToAdd = value & 0xFF;
                    int secondToAdd = (value>>8) & 0xFF;
                    value = (firstToAdd << 8) | secondToAdd;
                    to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16) | value;
                    this->section->add_content(4, to_insert);
                    sym->insert_flink(this->location_cnt,2,this->section,R_16);
                }
                else{
                    // symbol is not defined, we have to add FLINK
                    int value = 0;
                    to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
                    this->section->add_content(4, to_insert);
                    sym->insert_flink(this->location_cnt,2,this->section,R_16);
                }
            }
            else{
                // there is not this symbol
                /*
                Ako labela nije nađena u tabeli simbola, 
                biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                */
                Symbol* sym = new Symbol(sym_name,0,2,SimType_DATA,"",0,0,0);
                sym->insert_flink(this->location_cnt,2,this->section,R_16);
                this->list_symbols.push_back(sym);
                int value = 0;
                to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
                this->section->add_content(4, to_insert);
            }
            
        }
        this->location_cnt+=5;
        cout << "                                    loc_cnt jeeee (hex) " << this->location_cnt << endl;
        
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym)){
        // LDR: 1010 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL
        // STR: 1011 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL
        // ldr/str regD, sym
        this->location_cnt+=5;
        to_insert |= (regD << 28) | (15 << 24) | (4 << 16);
        smatch m;
        regex_match(line,m,reg_ins_ldr_str_sym);
        for(int i = 0; i<10; i++){
            //cout<< "                                                       m.str(" << i <<"): " << m.str(i) << endl;
        }
        string symbol=get_operand_ldr_str(line);
        this->out1 << "Symbol in ldr/str regD, sym is: " << symbol << endl;
        Symbol* sym = Symbol::get_symbol_by_name(symbol);
            if(sym != nullptr){
                // there is this symbol
                if(sym->is_defined()){
                    // definisan simbol, imamo vrednost
                    // ugradi vrednost
                    int value = sym->get_value();
                    int firstToAdd = value & 0xFF;
                    int secondToAdd = (value>>8) & 0xFF;
                    value = (firstToAdd << 8) | secondToAdd;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
                }
                else{
                    // symbol is not defined, we have to add FLINK
                    int value = 0;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
                }
            }
            else{
                // there is not this symbol
                /*
                Ako labela nije nađena u tabeli simbola, 
                biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                */
                Symbol* sym = new Symbol(symbol,0,2,SimType_DATA,"",0,0,0);
                sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
                this->list_symbols.push_back(sym);
            }
        this->section->add_content(4,to_insert);
        this->out1 << "regD, symbol: " << regD << " " << symbol <<endl;
        this->out1 << "Ldr/str regD, symbol; to_insert should be 1010(1011) 0000 DDDD 1111 0000 0100 HHHH HHHH LLLL LLLL, and it is: " << std::hex << to_insert<< endl;
    
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym_dollar)){
        // LDR: 1010 0000 DDDD 1111 0000 0000 HHHH HHHH LLLL LLLL
        // STR: 1011 0000 DDDD 1111 0000 0000 HHHH HHHH LLLL LLLL
        // ldr/str regD, $sym  
        this->location_cnt+=5;
        string sym_name=get_operand_ldr_str(line);
        this->out1 << "Symbol in ldr/str regD, $sym is: " << sym_name << endl;
        to_insert |= (regD << 28) | (0xF << 24);
        to_insert |= (0x00 << 16);

        Symbol* sym = Symbol::get_symbol_by_name(sym_name);
        if(sym != nullptr){
                // there is this symbol
                if(sym->is_defined()){
                    // definisan simbol, imamo vrednost
                    // ugradi vrednost
                    int value = sym->get_value();
                    int firstToAdd = value & 0xFF;
                    int secondToAdd = (value>>8) & 0xFF;
                    value = (firstToAdd << 8) | secondToAdd;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
                }
                else{
                    // symbol is not defined, we have to add FLINK
                    int value = 0;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
                }
            }
        else{
            // there is not this symbol
            /*
            Ako labela nije nađena u tabeli simbola, 
            biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
            Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
            U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
            tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
            */
            Symbol* sym = new Symbol(sym_name,0,2,SimType_DATA,"",0,0,0);
            sym->insert_flink(this->location_cnt-2,2,this->section,R_16);
            this->list_symbols.push_back(sym);
        }

        this->section->add_content(4,to_insert);
        this->out1 << "regD, sym: " << regD << " " << sym_name <<endl;
        this->out1 << "Ldr/str regD, $sym; to_insert should be 1010(1011) 0000 DDDD 1111 0000 0000 HHHH HHHH LLLL LLLL, and it is: " << std::hex << to_insert << endl;
    
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym_percent)){
        // LDR: 1010 0000 DDDD 0111 0000 0101 HHHH HHHH LLLL LLLL
        // STR: 1011 0000 DDDD 0111 0000 0101 HHHH HHHH LLLL LLLL
        this->location_cnt+=5;
        int addr_mode = 0x03; // regS is PC (r7) + operand, regind with add
        int regS = 7;
        to_insert |= (regD << 28) | (regS << 24) | (addr_mode << 16);
        
        string symbol = get_operand_ldr_str(line);
        Symbol* sym = Symbol::get_symbol_by_name(symbol);
            if(sym != nullptr){
                // there is this symbol
                if(sym->is_defined()){
                    // definisan simbol, imamo vrednost
                    // ugradi vrednost
                    int value = sym->get_value();
                    int firstToAdd = value & 0xFF;
                    int secondToAdd = (value>>8) & 0xFF;
                    value = (firstToAdd << 8) | secondToAdd;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16PC);
                }
                else{
                    // symbol is not defined, we have to add FLINK
                    int value = 0;
                    to_insert |= value;
                    sym->insert_flink(this->location_cnt-2,2,this->section,R_16PC);
                }
            }
            else{
                // there is not this symbol
                /*
                Ako labela nije nađena u tabeli simbola, 
                biće kreiran novi ulaz za nju, postavljeno polje name i polje defined = false. 
                Polje flink se postavlja da pokazuje na novo kreirani ulaz u tabeli obraćanja unapred. 
                U tom novom ulazu u polje byte upisuje se tekuća vrednost brojača lokacija LC 
                tj. adresa u mašinskom kodu koja odgovara adresnom polju tekuće instrukcije.
                */
                Symbol* sym = new Symbol(symbol,0,2,SimType_DATA,"",0,0,0);
                sym->insert_flink(this->location_cnt-1,2,this->section,R_16PC);
                this->list_symbols.push_back(sym);
            }
        this->section->add_content(4,to_insert);
        this->out1 << "regD, %symbol: " << regD << " " << symbol <<endl;
        this->out1 << "Ldr/str regD, %symbol; to_insert should be 1010(1011) 0000 DDDD 0111 0000 0101 HHHH HHHH LLLL LLLL, and it is: " << std::hex << to_insert<< endl;
    
    }
}


#pragma region auxiliary_methods_for_detecting_directives_and_orders

int Assembler::is_comment(string word)
{
    smatch m;
    if (regex_match(word,m,regex_comment))
        return 1;
    else 
        return 0;
}

int Assembler::is_label_only(string word)
{
    smatch m;
    if( regex_match(word,m,reg_label_itself) )
        return 1;
    else
        return 0;
}

int Assembler::is_label_with_sth(string word)
{
    smatch m;
    if( regex_match(word,m,reg_label_and_sth) )
        return 1;
    else
        return 0;
}

Directive Assembler::is_directive(string word)
{
    //this->out1 <<"check:" << word << "pattern: " << reg_dir_global_str << endl;
    smatch m;
    if( regex_search(word,m,reg_dir_global) )
    {
        return _global;
    }
    else if( regex_search(word,m,reg_dir_extern) )
    {
        return _extern;
    }
    else if( regex_search(word,m,reg_dir_section) )
    {
        return _section;
    }
    else if( regex_search(word,m,reg_dir_word) )
    {
        return _word;
    }
    else if( regex_search(word,m,reg_dir_skip) )
    {
        return _skip;
    }
    else if( regex_search(word,m,reg_dir_equ) )
    {
        return _equ;
    }
    else if( regex_search(word,m,reg_dir_end) )
    {
        return _end;
    }
    else if( regex_search(word,m,reg_dir_ascii) )
    {
        return _ascii;
    }
    else
    {
        return _no_dir;
    }
}

Instruction Assembler::is_instruction(string word)
{
    smatch m;
    if( regex_search(word,m,reg_ins_halt) )
    {
        return HALT;
    }

    if( regex_search(word,m,reg_ins_iret) )
    {
        return IRET;
    }
    if( regex_search(word,m,reg_ins_ret) )
    {
        return RET;
    }
    if( regex_search(word,m,reg_ins_call_lit)| 
    regex_search(word,m,reg_ins_call_reg) |
    regex_search(word,m,reg_ins_call_reg_mem) |
    regex_search(word,m,reg_ins_call_reg_plus_lit) |
    regex_search(word,m,reg_ins_call_reg_plus_sym) |
    regex_search(word,m,reg_ins_call_sym) |
    regex_search(word,m,reg_ins_call_sym_percent) |
    regex_search(word,m,reg_ins_call_lit_ptr) |
    regex_search(word,m,reg_ins_call_sym_ptr))
    {
        this->out1 << "line with 'call' is: "<<word << endl;
        return CALL;
    }
    if( regex_search(word,m,reg_ins_ldr_str_lit) |
    regex_search(word,m,reg_ins_ldr_str_lit_dollar) |
    regex_search(word,m,reg_ins_ldr_str_reg) |
    regex_search(word,m,reg_ins_ldr_str_reg_mem) |
    regex_search(word,m,reg_ins_ldr_str_reg_plus_lit) |
    regex_search(word,m,reg_ins_ldr_str_reg_plus_sym) |
    regex_search(word,m,reg_ins_ldr_str_sym) |
    regex_search(word,m,reg_ins_ldr_str_sym_dollar) |
    regex_search(word,m,reg_ins_ldr_str_sym_percent) )
    {
        string inst=get_name_of_instruction_two_reg(word);
        if(inst.compare("ldr")==0) return LDR;
        else return STR;
    }
    if( regex_search(word,m,reg_ins_one_reg) )
    {
        string inst=get_name_of_instruction_one_reg(word);
        if(inst.compare("int")==0) return INT;
        if(inst.compare("push")==0) return PUSH;
        if(inst.compare("pop")==0) return POP;
        if(inst.compare("not")==0) return NOT;
    }
    if( regex_search(word,m,reg_ins_two_reg) )
    {
        string inst=get_name_of_instruction_two_reg(word);
        if(inst.compare("xchg")==0) return XCHG;
        if(inst.compare("add")==0) return ADD;
        if(inst.compare("sub")==0) return SUB;
        if(inst.compare("mul")==0) return MUL;
        if(inst.compare("div")==0) return DIV;
        if(inst.compare("cmp")==0) return CMP;
        if(inst.compare("and")==0) return AND;
        if(inst.compare("or")==0) return OR;
        if(inst.compare("xor")==0) return XOR;
        if(inst.compare("test")==0) return TEST;
        if(inst.compare("shl")==0) return SHL;
        if(inst.compare("shr")==0) return SHR;
    }
    if( regex_search(word,m,reg_ins_jmp_lab_or_lit) | 
    regex_search(word,m,reg_ins_jmp_pc_rel) | 
    regex_search(word,m,reg_ins_jmp_ptr_lit_or_sym) | 
    regex_search(word,m,reg_ins_jmp_ptr_reg) | 
    regex_search(word,m,reg_ins_jmp_ptr_with_add))
    {
        string inst=get_name_of_instruction_one_reg(word);
        this->out1 << "line with 'jmp' is: "<<word << endl;
        this->out1 << "instruction is: "<< inst << endl;
        if(inst.compare("jmp")==0) return JMP;
        if(inst.compare("jeq")==0) return JEQ;
        if(inst.compare("jne")==0) return JNE;
        if(inst.compare("jgt")==0) return JGT;
    }
    return NO_INST;
}

#pragma endregion


#pragma region auxiliary_functions_for_work_with_strings

string Assembler::remove_spaces_from_string(string line){
    string str;
    string to_add;

    for (int i = 0; i<line.length(); i++)
        if (line[i] != ' ' && line[i] != '\t'){
        	to_add=line[i];
        	str.append(to_add);
        }

    this->out1 << "after remove_spaces_from_string: " << str << endl;

    return str;
}
string Assembler::remove_leading_trailing_spaces(string line){
	int ind0=0;
	int ind1=line.length()-1;
	if( line.find_first_not_of(" ") != string::npos){
		ind0=line.find_first_not_of(" ");
	}
	if( line.find_last_not_of(" ") != string::npos){
		ind1=line.find_last_not_of(" ");
	}

	return (line.substr(ind0, ind1-ind0+1));
}
string Assembler::remove_surplus_spaces(string line){
    string result = remove_leading_trailing_spaces(line);

    int beginSpace = result.find_first_of(" ");
    while (beginSpace != std::string::npos)
    {
    	int endSpace = result.find_first_not_of(" ", beginSpace);
    	int range = endSpace - beginSpace;

        result.replace(beginSpace, range, " ");

        int newStart = beginSpace + 1;
        beginSpace = result.find_first_of(" ", newStart);
    }

    return result;
}
string Assembler::get_name_after_directive(string line){
    //.equ PRINT_REG, 0xFF00
    string string_for_reg = "^(( )|(\t))*(.global|.extern|.section|.word|.skip|.equ|.end|.ascii)( )+(.+)((( )|(\t))*#.*)*$";
    regex reg = regex(string_for_reg);
    smatch m;
    bool sth = regex_match(line,m,reg);
    if(sth ==  false) return "";
    string name = m.str(6);
    this->out1 << "after directive is: #" << name << "#" << endl;
    return name;
}
string Assembler::get_string_after_instruction(string inst){
    string string_for_reg = "^(( )|(\t))*(call|jmp|jeq|jne|jgt)( )+(.+)((( )|(\t))*#.*)*$";
    regex reg = regex(string_for_reg);
    smatch m;
    bool sth = regex_match(inst,m,reg);
    if(sth ==  false) return "";
    string name = m.str(6);
    this->out1 << "after instruction is: " << name << endl;
    return name;
}
string Assembler::get_name_of_label(string line){
    string name = "";
    smatch m;
    if(regex_match(line,m,reg_label_itself)){
        name = m.str(4);
        this->out1 << "label is: " << name << endl;
        return name;
    }
    else if(regex_match(line,m,reg_label_and_sth)){
        name = m.str(4);
        this->out1 << "label is: " << name << endl;
        return name;
    }
    else return "";
}
string Assembler::get_name_after_label(string line){
    string name = "";
    smatch m;
    if(regex_match(line,m,reg_label_and_sth)){
        name = m.str(6);
        name=remove_surplus_spaces(name);
        this->out1 << "after instruction is: " << name << endl;
        return name;
    }
    else return "";
}
string Assembler::get_name_from_equ(string line){
    // .equ PRINT_REG, 0xFF00

    smatch m;
    bool sth = regex_match(line,m,reg_dir_equ);
    if(sth ==  false) return "";
    string to_ret = m.str(4);
    return to_ret;
}
string Assembler::get_literal_from_equ(string line){
    smatch m;
    bool sth = regex_match(line,m,reg_dir_equ);
    if(sth ==  false) return "";
    string to_ret = m.str(6);
    return to_ret;
}
string Assembler::get_name_of_instruction_two_reg(string line){
    string string_for_reg = "^(( )|(\t))*(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr|ldr|str)( )+(( )*(r([0-7]|sp|pc)|psw)( )*, ( )*(.+)( )*)( )*((( )|(\t))*#.*)*$";
    regex reg = regex(string_for_reg);
    smatch m;
    bool sth = regex_match(line,m,reg);
    if(sth ==  false) return "";
    string name = m.str(4);
    this->out1 << "name of instruction (m.str(4)) is " << name << endl;
    return name;
}
string Assembler::get_name_of_instruction_one_reg(string line){
    string name = "";
    smatch m;
    smatch m1;
    regex reg = regex("^(( )|(\t))*(jmp|jeq|jne|jgt)( )+.*((( )|(\t))*#.*)*$");
    if(regex_match(line,m1,reg)){
        name = m1.str(4);
        this->out1 << "name of instruction (m.str(4)) is " << name << endl;
        return name;
    }
    bool sth = regex_match(line,m,this->reg_ins_one_reg);
    if(sth ==  false) return "";
    name = m.str(4);
    this->out1 << "name of instruction (m.str(4)) is " << name << endl;
    return name;
}
int Assembler::get_num_of_symbols_in_string_with_commas(string line){
    if(line.find_first_of(',')==string::npos) return 0;
    else{
        this->out1 << line << endl;
        int cnt=0;
        for(int i=0;i<line.length();i++){
            if(line[i]==','){
                cnt++;
            }
        }
        return (cnt+1);
    }
}
int Assembler::get_numerical_value_from_string(string line){

    smatch m;
    if(regex_search(line,m,regex_literal) == false) return INT32_MIN;
    line = remove_leading_trailing_spaces(line);
    
    //cout << line + " is line"<< endl; 
    int x=-1;
    if(line[0]=='0' && line[1]=='x'){
        cout << line << endl;        
        x=std::stoul(line,nullptr,16);
    }
    else{
        x=std::stoul(line,nullptr,10);
    }
    this->out1 << "numerical value from string '" << line << "' is: " << hex << x  << " (" << dec << x << ")" << endl;
    return x;
}
int Assembler::get_num_of_bytes_in_integer(int x){
    int n = 0;
    while (x != 0) {
        x >>= 8;
        n ++;
    }
    return n;
}
int Assembler::get_hex_values_from_char(char name)
{
    char buff[2];
    sprintf(buff, "%X", name);
    string s =buff;
    if(s.length()==1) s = "0" + s;
    s = "0x" + s;
    int toRet = get_numerical_value_from_string(s);
    this->out1 << "name in the form of a string is: " << name << ", and returned ascii value is: " << toRet << endl;
    return toRet;
}
#pragma endregion


#pragma region auxiliary_functions_for_conversion_instructions_in_machine_code

int Assembler::get_instr_descr(Instruction inst){
    switch (inst)
    {
    case HALT: return 0x00;
    case INT: return 0x10;
    case IRET: return 0x20;
    case CALL: return 0x30;
    case RET: return 0x40;
    case JMP: return 0x50;
    case JEQ: return 0x51;
    case JNE: return 0x52;
    case JGT: return 0x53;
    case PUSH: return 0xB0;
    case POP: return 0xA0;
    case XCHG: return 0x60;
    case ADD: return 0x70;
    case SUB: return 0x71;
    case MUL: return 0x72;
    case DIV: return 0x73;
    case CMP: return 0x74;
    case NOT: return 0x80;
    case AND: return 0x81;
    case OR: return 0x82;
    case XOR: return 0x83;
    case TEST: return 0x84;
    case SHL: return 0x90;
    case SHR: return 0x91;
    case LDR: return 0xA0;
    case STR: return 0xB0;
    default: return -1;
    }
}

int Assembler::get_num_of_reg_one_reg_inst(string line){
    smatch m;
    line = this->remove_surplus_spaces(line);
    this->out1 << line << endl;
    regex_match(line,m,reg_ins_one_reg);
    string regD=m.str(6);
    // for(int i = 0; i<10; i++){
    //     this->out1 << "                                                       m.str(" << i <<"): " << m.str(i) << endl;
    // }
    int to_ret = -1;
    if(regD.find("sp") != string::npos) return 6;
    if(regD.find("pc") != string::npos) return 7;
    if(regD.find("psw") != string::npos) return 8;
    to_ret = stoi(m.str(7));
    this->out1 << "Num of reg in 1-byte instr is: " << to_ret;
    return to_ret;
}

int Assembler::get_num_of_first_reg_two_reg_inst(string line){
    smatch m;
    line = this->remove_surplus_spaces(line);
    this->out1 << line << endl;
    regex_match(line,m,reg_ins_two_reg);
    string regD=m.str(6);
    // for(int i = 0; i<10; i++){
    //     this->out1 << "                                                       m.str(" << i <<"): " << m.str(i) << endl;
    // }
    int to_ret = -1;
    if(regD.find("sp") != string::npos) return 6;
    if(regD.find("pc") != string::npos) return 7;
    if(regD.find("psw") != string::npos) return 8;
    to_ret = stoi(m.str(7));
    this->out1 << "Num of first reg in 2-byte instr is: " << regD.substr(1) << endl;
    return to_ret;
}

int Assembler::get_num_of_second_reg_two_reg_inst(string line){
    smatch m;
    line = this->remove_surplus_spaces(line);
    this->out1 << line << endl;
    regex_match(line,m,reg_ins_two_reg);
    string regD=m.str(10);
    // for(int i = 0; i<10; i++){
    //     this->out1 << "                                                       m.str(" << i <<"): " << m.str(i) << endl;
    // }
    int to_ret = -1;
    if(regD.find("sp") != string::npos) return 6;
    if(regD.find("pc") != string::npos) return 7;
    if(regD.find("psw") != string::npos) return 8;
    to_ret = stoi(m.str(11));
    this->out1 << "Num of second reg in 2-byte instr is: " << regD.substr(1) << endl;
    return to_ret;
}

int Assembler::get_num_of_reg_jmps(string line){
    cout << "u get_num_of_reg_jmps smo za liniju " << line << endl;
    smatch m;
    int i1 = 0;
    int i2 = 0;
    string r;
    line = this->remove_surplus_spaces(line);
    this->out1 << line << endl;
    if(regex_match(line,m,reg_ins_jmp_ptr_with_add) | 
    regex_match(line,m,reg_ins_call_reg_plus_lit) | 
    regex_match(line,m,reg_ins_call_reg_plus_sym) | 
    regex_match(line,m,reg_ins_ldr_str_reg_plus_lit) | 
    regex_match(line,m,reg_ins_ldr_str_reg_plus_sym)){
        // [reg + lit/sym]
        line = this->remove_spaces_from_string(line);
        i1 = line.find_first_of("[");
        i2 = line.find_first_of("+");
        r = line.substr(i1+1,i2-i1-1);
        if(r.find("sp") != string::npos) return 6;
        if(r.find("pc") != string::npos) return 7;
        if(r.find("psw") != string::npos) return 8;
    }
    else if(line.find_first_of("[") != string::npos){
        // *[reg]
        line = this->remove_spaces_from_string(line);
        i1 = line.find_first_of("[");
        i2 = line.find_first_of("]");
        r = line.substr(i1+1,i2-i1-1);
        if(r.find("sp") != string::npos) return 6;
        if(r.find("pc") != string::npos) return 7;
        if(r.find("psw") != string::npos) return 8;
    }
    else{
        // *reg
        line = this->remove_spaces_from_string(line);
        i1 = line.find_first_of("*");
        r = line.substr(i1+1);  
        if(r.find("sp") != string::npos) return 6;
        if(r.find("pc") != string::npos) return 7;
        if(r.find("psw") != string::npos) return 8;
    }
    this->out1 << "In line: " << line << " reg is " << r << endl;
    r = "push " + r;
    this->out1 << "We will find out num of reg from instr: " << r << endl;
    return get_num_of_reg_one_reg_inst(r);
}

int Assembler::get_num_of_regD_ldr_str(string line){
    smatch m;
    this->out1 << "u get_num_of_reg_ldr_str smo" << endl;
    if(regex_search(line,m,reg_ins_ldr_str_lit_dollar)){
        return find_regD_ldr_str(line,reg_ins_ldr_str_lit_dollar);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym_dollar  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_sym_dollar);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_lit  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_lit);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_sym);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym_percent   ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_sym_percent);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_reg);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg_mem  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_reg_mem);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg_plus_lit   ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_reg_plus_lit);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_reg_plus_sym  ))
    {
        return find_regD_ldr_str(line,reg_ins_ldr_str_reg_plus_sym);
    }
    return -1;
}

string Assembler::get_operand_ldr_str(string line)
{
    smatch m;
    string to_ret = "";
    if(regex_match(line,m,this->reg_ins_ldr_str_reg_plus_lit) || regex_match(line,m,this->reg_ins_ldr_str_reg_plus_sym)){
        to_ret = m.str(18);
    }
    else if(regex_match(line,m,this->reg_ins_ldr_str_reg) || regex_match(line,m,this->reg_ins_ldr_str_reg_mem)){
        to_ret = m.str(13);
        if(to_ret.find("sp") != string::npos) return "6";
        if(to_ret.find("pc") != string::npos) return "7";
        if(to_ret.find("psw") != string::npos) return "8";
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym_percent)){
        // this->out1 << "m.str(10): " << m.str(12)<<endl;
        to_ret = m.str(12);
    }
    else if(regex_search(line,m,reg_ins_ldr_str_sym  ) ||
    regex_search(line,m,reg_ins_ldr_str_lit  ) ||
    regex_search(line,m,reg_ins_ldr_str_sym_dollar  ) ||
    regex_search(line,m,reg_ins_ldr_str_lit_dollar)){
        to_ret = m.str(11);
    }
    this->out1 << "Operand in ldr/str instr is: " << to_ret << endl;
    return to_ret;
}

int Assembler::find_regD_ldr_str(string line,regex r)
{
    smatch m;
    regex_match(line,m,r);
    int i = 0;
    // for(i; i<10; i++){
    //     this->out1 << "                                                       m.str(" << i <<"): " << m.str(i) << endl;
    // }
    // this->out1 << "m.str(i) jee " << m.str(i);
    string to_ret_str = m.str(8);
    if(to_ret_str.find("sp") != string::npos) return 6;
    if(to_ret_str.find("pc") != string::npos) return 7;
    if(to_ret_str.find("psw") != string::npos) return 8;
    to_ret_str = m.str(9);
    int to_ret = stoi(to_ret_str);
    this->out1 << "Num of first reg in ldr/str instr is: " << to_ret << endl;
    return to_ret;
}

string Assembler::get_lit_ot_sym_from_jmps_with_add(string line){
    cout << "u get_lit_ot_sym_from_jmps_with_add smo za liniju " << line << endl;

    smatch m;
    int i1 = 0;
    int i2 = 0;
    string r;
    this->out1 << line << endl;
    if(regex_match(line,m,reg_ins_jmp_ptr_with_add) | 
    regex_match(line,m,reg_ins_call_reg_plus_lit) | 
    regex_match(line,m,reg_ins_call_reg_plus_sym) | 
    regex_match(line,m,reg_ins_ldr_str_reg_plus_lit) | 
    regex_match(line,m,reg_ins_ldr_str_reg_plus_sym)){
        // [reg + lit/sym]
        line = this->remove_spaces_from_string(line);
        i1 = line.find_first_of("+");
        i2 = line.find_first_of("]");
        cout << "i1: " << i1 << ", i2: " << i2 << endl;
        r = line.substr(i1+1,i2-i1-1);
    }
    this->out1 << "In line: " << line << " lit/sym is " << r << endl;
    return r;
}

#pragma endregion


void Assembler::write_sym_table()
{
    
    int width = 20;
    char sep = ' ';

    this->out1<< endl;
    this->out1 << right << std::setw(57) << std::setfill(sep) << "SIMBOL TABLE";
    this->out1 << left << std::setw(width*5+6) << std::setfill('*') << endl;
    this->out1 << "\n";
    this->out1 << left << std::setw(width+2) << std::setfill(sep) << std::hex << "\n|ID";
    this->out1 << left << std::setw(width) << std::setfill(sep) <<  "NAME";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) <<  "VALUE";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) << "SECTION";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) << "BINDING";
    this->out1 << left << "|" << endl;
    this->out1 << endl;

    cout<< endl;
    cout << right << std::setw(57) << std::setfill(sep) << "SIMBOL TABLE";
    cout << left << std::setw(width*5+6) << std::setfill('*') << endl;
    cout << "\n";
    cout << left << std::setw(width+2) << std::setfill(sep) << std::hex << "\n|ID";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "NAME";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "VALUE";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "SECTION";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "BINDING";
    cout << left << "|" << endl;
    cout << endl;

    for(auto iter = this->list_symbols.begin(); iter != this->list_symbols.end(); iter++)
    {
        Symbol* sym= *iter;
        cout << *sym;
        out1 << *sym << endl;
        
    }
    this->out1 << left << std::setw(width*5+6) << std::setfill('*') << "\n" << endl;
    cout << left << std::setw(width*5+6) << std::setfill('*') << "\n" << endl;

    for(auto iter = this->list_sections.begin(); iter != this->list_sections.end(); iter++)
    {
        Section* sym= *iter;
        cout << *sym;
        out1 << *sym << endl;
    }

    this->out1<< endl;
    this->out1 << right << std::setw(57) << std::setfill(sep) <<  "RELOCATION RECORDS";
    this->out1 << left << std::setw(width*5+6) << std::setfill('*') << endl;
    this->out1 << "\n";
    this->out1 << left << std::setw(width+2) << std::setfill(sep) << std::hex << "\n|Section";
    this->out1 << left << std::setw(width) << std::setfill(sep) <<  "Offset";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) <<  "Type";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) << "Name";
    this->out1 << left << "|";
    this->out1 << left << std::setw(width) << std::setfill(sep) << "Addend";
    this->out1 << left << "|" << endl;
    this->out1 << endl;

    cout<< endl;
    cout << right << std::setw(57) << std::setfill(sep) << "RELOCATION RECORDS";
    cout << left << std::setw(width*5+6) << std::setfill('*') << endl;
    cout << "\n";
    cout << left << std::setw(width+2) << std::setfill(sep) << std::hex << "\n|Section";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "Offset";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) <<  "Type";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "Name";
    cout << left << "|";
    cout << left << std::setw(width) << std::setfill(sep) << "Addend";
    cout << left << "|" << endl;
    cout << endl;

    for(auto iter = this->list_sections.begin(); iter != this->list_sections.end(); iter++)
    {
        Section* sym= *iter;
        for(auto iter1 = sym->list_of_my_relocation_records.begin(); iter1 != sym->list_of_my_relocation_records.end(); iter1++)
        {
            out1 << "pisem relokacioni zapis" << endl;
            RelocationRecord* sym1= *iter1;
            cout << *sym1;
            out1 << *sym1 << endl;
        }
    }

    this->out1 << left << std::setw(width*5+6) << std::setfill('*') << "\n" << endl;
    cout << left << std::setw(width*5+6) << std::setfill('*') << "\n" << endl;
}

void Assembler::generateObjFile(string filename){
    ofstream os;
    os.open(filename,ios::out);
    int width = 20;
    char sep = ' ';

    if(!this->list_symbols.empty()) {
        os << "Symbol table" << endl;
        os << left << std::setw(width+1) << std::setfill(sep) << std::hex << "|ID";
        os << left << "|";
        os << left << std::setw(width) << std::setfill(sep) <<  "NAME";
        os << left << "|";
        os << left << std::setw(width) << std::setfill(sep) <<  "VALUE";
        os << left << "|";
        os << left << std::setw(width) << std::setfill(sep) << "SECTION";
        os << left << "|";
        os << left << std::setw(width) << std::setfill(sep) << "BINDING";
        os << left << "|" << endl;
    }
    for(auto iter = this->list_symbols.begin(); iter != this->list_symbols.end(); iter++)
    {
        Symbol* sym= *iter;
        os << *sym;
    }

    int has_rel_rec = 0;
    for(auto iter = this->list_sections.begin(); iter != this->list_sections.end(); iter++)
    {
        Section* sym= *iter;
        if(!sym->list_of_my_relocation_records.empty() && has_rel_rec==0){
            has_rel_rec = 1;
            os << "\nRelocation records" << endl;
            os << left << std::setw(width+2) << std::setfill(sep) << std::hex << "|Section";
            os << left << std::setw(width) << std::setfill(sep) <<  "Offset";
            os << left << "|";
            os << left << std::setw(width) << std::setfill(sep) <<  "Type";
            os << left << "|";
            os << left << std::setw(width) << std::setfill(sep) << "Name";
            os << left << "|";
            os << left << std::setw(width) << std::setfill(sep) << "Addend";
            os << left << "|" << endl;
        }
        for(auto iter1 = sym->list_of_my_relocation_records.begin(); iter1 != sym->list_of_my_relocation_records.end(); iter1++)
        {
            RelocationRecord* sym1= *iter1;
            os << *sym1;
        }
    }

    if(!this->list_sections.empty()) os << "\nSection content" << endl;
    for(auto iter = this->list_sections.begin(); iter != this->list_sections.end(); iter++)
    {
        Section* sym= *iter;
        os << *sym;
    }

    os.close();
}