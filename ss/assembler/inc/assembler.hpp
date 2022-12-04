#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>
#include <regex>
#include "symbol.hpp"
#include "section.hpp"
#include "relocationrecord.hpp"
using namespace std;

enum Directive
{
    _global,
    _extern,
    _section,
    _word,
    _skip,
    _equ,
    _end,
    _ascii,
    _no_dir
};

enum Instruction
{
    NO_INST,
    HALT,
    INT,
    IRET,
    CALL,
    RET,
    JMP,
    JEQ,
    JNE,
    JGT,
    PUSH,
    POP,
    XCHG,
    ADD,
    SUB,
    MUL,
    DIV,
    CMP,
    NOT,
    AND,
    OR,
    XOR,
    TEST,
    SHL,
    SHR,
    LDR,
    STR
};

class Assembler
{

public:

    Assembler(string file_in, string file_out);

    void single_pass_assembler();
    void backpatching(string file_in);
    void backpatching();
    void generateObjFile(string filename);

    std::list<Symbol> my_symbols;
    std::list<Section> my_sections;
    std::list<RelocationRecord> my_relocation_records;

    void update_symbol_table(string& line);
    void process_directive(Directive dir, int ser_num, string line);
    void process_instruction(Instruction instr, int ser_num, string line);
    void process_label(string label);
    void process_ldr_str_instruction(string line, Instruction instr);
    void write_sym_table();
    string remove_comment(string line);
    string get_name_after_directive(string dir);
    string get_string_after_instruction(string inst);
    string get_name_of_label_only(string line);
    string get_name_of_label(string line);
    string get_name_after_label(string line);
    string get_name_from_equ(string line);
    string get_literal_from_equ(string line);
    string get_name_of_instruction_two_reg(string line);
    string get_name_of_instruction_one_reg(string line);
    string remove_spaces_from_string(string line);
    string remove_leading_trailing_spaces(string line);
    string remove_surplus_spaces(string line);
    int get_num_of_symbols_in_string_with_commas(string line);
    int get_numerical_value_from_string(string line);
    int get_num_of_bytes_in_integer(int x);
    int get_hex_values_from_char(char name);
    int get_instr_descr(Instruction inst);
    int get_num_of_reg_one_reg_inst(string line);
    int get_num_of_first_reg_two_reg_inst(string line);
    int get_num_of_second_reg_two_reg_inst(string line);
    int get_num_of_reg_jmps(string line);
    string get_operand_ldr_str(string line);
    int get_num_of_regD_ldr_str(string line);
    int find_regD_ldr_str(string line,regex r);
    string get_lit_ot_sym_from_jmps_with_add(string line);

    Directive is_directive(string word);
    Instruction is_instruction(string word);
    int is_label_only(string word);
    int is_label_with_sth(string word);
    int is_comment(string word);

    std::string reg_sym;
    std::string reg_num;
    std::string reg_hex;
    std::string reg_ascii;
    std::string reg_lit;
    std::string reg_sym_lit;
    std::string reg_sym_list;
    std::string reg_lit_list;
    std::string reg_sym_lit_list;
    std::string reg_register; // ( )*(r([0-7]|sp|pc)|psw)( )*
    std::string reg_register_ind; // ^( )*(\[(r([0-7]|sp|pc)|psw)\])( )*$
    std::string reg_reg_plus_lit; // [reg+7]
    std::string reg_reg_plus_sym; // [reg+gaga]
    std::string reg_reg_lit_dollar; // $lit
    std::string reg_reg_sym_dollar; // $sym
    std::string reg_reg_lit_ptr; // *lit
    std::string reg_reg_sym_ptr; // *sym
    std::string reg_reg_sym_percent; // %sym
    std::string reg_operand_jmp_label_or_literal; // jmp literal | jmp sym
    std::string reg_operand_jmp_pc_rel; // jmp %sym
    std::string reg_operand_jmp_ptr_reg; // jmp *reg | jmp *[reg]
    std::string reg_operand_jmp_ptr_lit_or_sym; // jmp *literal | jmp *sym
    std::string reg_operand_jmp_ptr_with_add; // jmp *[reg+literal] | jmp *[reg+sym]
    std::string reg_operand_data; // $lit|$sim|lit|sim|%sim|reg|[reg]|[reg+lit]|[reg+sim]
    std::string reg_dir_global_str;
    std::string reg_inst_halt;
    std::string reg_inst_iret;
    std::string reg_inst_ret;
    std::string reg_inst_one_registar;
    std::string reg_inst_two_registar;
    std::string reg_operand_call_lit_ptr;
    std::string reg_operand_call_sym_ptr;
    std::string reg_operand_call_lit;
    std::string reg_operand_call_sym;
    std::string reg_operand_call_sym_percent;
    std::string reg_operand_call_reg;
    std::string reg_operand_call_reg_mem;
    std::string reg_operand_call_reg_plus_lit;
    std::string reg_operand_call_reg_plus_sym;
    std::string reg_operand_ldr_str_lit_dollar;
    std::string reg_operand_ldr_str_sym_dollar;
    std::string reg_operand_ldr_str_lit;
    std::string reg_operand_ldr_str_sym;
    std::string reg_operand_ldr_str_sym_percent;
    std::string reg_operand_ldr_str_reg;
    std::string reg_operand_ldr_str_reg_mem;
    std::string reg_operand_ldr_str_reg_plus_lit;
    std::string reg_operand_ldr_str_reg_plus_sym;

    regex regex_comment;
    regex regex_symbol;
    regex regex_literal;
    regex regex_literal_dec;
    regex regex_literal_hex;
    regex regex_literal_decONLY;
    regex regex_literal_hexONLY;
    regex reg_label_itself;
    regex reg_label_and_sth;
    regex reg_dir_global;
    regex reg_dir_extern;
    regex reg_dir_section;
    regex reg_dir_word;
    regex reg_dir_skip;
    regex reg_dir_ascii;
    regex reg_dir_equ;
    regex reg_dir_end;
    regex reg_ins_halt;
    regex reg_ins_iret;
    regex reg_ins_ret;
    regex reg_ins_one_reg;
    regex reg_ins_two_reg;
    regex reg_ins_jmp;
    regex reg_ins_ldr;
    regex reg_ins_str;
    regex reg_ins_call;
    regex reg_ins_call_lit_ptr;
    regex reg_ins_call_sym_ptr;
    regex reg_ins_call_lit;
    regex reg_ins_call_sym;
    regex reg_ins_call_sym_percent;
    regex reg_ins_call_reg;
    regex reg_ins_call_reg_mem;
    regex reg_ins_call_reg_plus_lit;
    regex reg_ins_call_reg_plus_sym;
    regex reg_ins_jmp_lab_or_lit; 
    regex reg_ins_jmp_pc_rel; 
    regex reg_ins_jmp_ptr_reg;
    regex reg_ins_jmp_ptr_lit_or_sym;
    regex reg_ins_jmp_ptr_with_add;
    regex reg_operand_jmp_reg_plus_lit;
    regex reg_operand_jmp_reg_plus_sym;
    regex reg_ins_ldr_str_lit_dollar;
    regex reg_ins_ldr_str_sym_dollar;
    regex reg_ins_ldr_str_lit;
    regex reg_ins_ldr_str_sym;
    regex reg_ins_ldr_str_sym_percent;
    regex reg_ins_ldr_str_reg;
    regex reg_ins_ldr_str_reg_mem;
    regex reg_ins_ldr_str_reg_plus_lit;
    regex reg_ins_ldr_str_reg_plus_sym;
    

private:

    int cnt;
    int end_single_assembling_process;
    int location_cnt;
    int error;
    string error_text;
    int num_curr_line;
    string curr_line;
    string curr_section_name;
    Section* section;
    ifstream* input_file;
    string out_file;
    ofstream out1;
    ofstream error_file;
    std::list<Symbol*> list_symbols;
    std::list<Section*> list_sections;


};


#endif