#include "riscv.h"

extern Register_physical reg[2][num_registers];
extern map<type_label, int>map_global_register;//全局寄存器名到编号的映射
extern map<int, Register_virtual*>map_global_register_position;//全局寄存器编号到指针的映射
extern map<int, Register_virtual*>map_local_register_position;//局部虚拟寄存器编号到指针的映射
extern map<int, std::string>map_physical_reg_name;//物理寄存器编号到名称的映射
extern vector<bool>physical_reg_usable[2];//物理寄存器能否使用
extern vector<int>physical_reg_order[2];//考虑分配物理寄存器的顺序
extern vector<int>physical_reg_saved[2];//special,caller_saved,callee_saved
extern int total_register;

extern variable_table* global, * global_tail;
extern int total_global;//存储变量的数量
extern map<type_variables, int>map_global;//变量名到编号的映射
extern map<int, variable_table*>map_variable_position;//局部变量编号到指针的映射
extern map<int, int>map_register_local;//寄存器编号到变量编号的映射

extern instruction* start;//属于全局的指令
extern map<std::string, int>ins_num, cond_num;
extern int tot_instructions;//总的指令数
extern int tot_asm_instructions;//总的汇编指令数
extern set<int>ins_definied;//会定义虚拟寄存器的指令(除call)
extern set<int>ins_used;//会使用虚拟寄存器的指令
extern set<int>ins_valuate;//会对Rd赋值的指令
extern map<int, instruction*>map_instruction_position;//指令编号到指针的映射
extern map<int, pair<int, int> >map_ins_copy;//记录copy指令的位置
extern map<int, std::string>map_asm;//汇编指令代码到字符映射

extern map<type_label, int>label_num;
extern map<int, int>label_ins;//label到指令的映射
extern vector<basic_block*>label_bb;//label编号对应的bb编号
extern int tot_label;

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern map<int, functions*>map_function_position;//根据函数编号查找函数指针
extern int tot_functions;//总的函数数