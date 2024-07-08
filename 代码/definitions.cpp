#include "riscv.h"

extern int num_start_func;//���������
extern Register_physical reg[2][num_registers];
extern map<type_label, int>map_global_register;//ȫ�ּĴ���������ŵ�ӳ��
extern map<int, Register_virtual*>map_global_register_position;//ȫ�ּĴ�����ŵ�ָ���ӳ��
extern map<int, Register_virtual*>map_local_register_position;//�ֲ�����Ĵ�����ŵ�ָ���ӳ��
extern map<int, std::string>map_physical_reg_name;//����Ĵ�����ŵ����Ƶ�ӳ��
extern vector<bool>physical_reg_usable[2];//����Ĵ����ܷ�ʹ��
extern vector<int>physical_reg_order[2];//���Ƿ�������Ĵ�����˳��
extern vector<int>physical_reg_saved[2];//special,caller_saved,callee_saved
extern int total_register;

extern variable_table* global, * global_tail;
extern int total_global;//�洢����������
extern map<type_variables, int>map_global;//����������ŵ�ӳ��
extern map<int, variable_table*>map_variable_position;//�ֲ�������ŵ�ָ���ӳ��
extern map<int, int>map_register_local;//�Ĵ�����ŵ�������ŵ�ӳ��

extern instruction* start;//����ȫ�ֵ�ָ��
extern map<std::string, int>ins_num, cond_num;
extern int tot_instructions;//�ܵ�ָ����
extern int tot_asm_instructions;//�ܵĻ��ָ����
extern set<int>ins_definied;//�ᶨ������Ĵ�����ָ��(��call)
extern set<int>ins_used;//��ʹ������Ĵ�����ָ��
extern set<int>ins_valuate;//���Rd��ֵ��ָ��
extern set<int>asm_change_Rd;//��ı�Rd��ָ��
extern map<int, instruction*>map_instruction_position;//ָ���ŵ�ָ���ӳ��
extern map<int, pair<int, int> >map_ins_copy;//��¼copyָ���λ��
extern map<int, std::string>map_asm;//���ָ����뵽�ַ�ӳ��

extern map<type_label, int>label_num;
extern map<int, int>label_ins;//label��ָ���ӳ��
extern vector<basic_block*>label_bb;//label��Ŷ�Ӧ��bb���
extern int tot_label;

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern map<int, functions*>map_function_position;//���ݺ�����Ų��Һ���ָ��
extern int tot_functions;//�ܵĺ�����