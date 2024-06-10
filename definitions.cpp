#include "riscv.h"

extern Register reg[num_registers];

extern variable_table* global, * global_tail;
extern int total_global;//存储全局变量的数量
extern map<type_variables, int>map_global;//变量名到编号的映射

extern instruction* start;//属于全局的指令
extern map<std::string, int>ins_num, cond_num;
extern int tot_instructions;//总的指令数

extern map<type_label, int>label_num;
extern int tot_label;

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern int tot_functions;//总的函数数
