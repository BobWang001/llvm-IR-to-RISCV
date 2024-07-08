#pragma once
#include <iostream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <climits>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include <stack>
#include <queue>
/*type of variables*/
#define type_variables std::string
#define type_registers int
#define type_label std::string
/*type of spilting weight*/
#define type_spliting_weight long long
/*type of register typies*/
#define i32 0
#define float32 1
#define i64 2
/*type of instructions*/
#define ins_global 0
#define ins_load 1
#define ins_store 2
#define ins_alloca 3
#define ins_getelementptr 4
#define ins_add 5
#define ins_fadd 6
#define ins_sub 7
#define ins_fsub 8
#define ins_mul 9
#define ins_fmul 10
#define ins_sdiv 11
#define ins_fdiv 12
#define ins_srem 13
#define ins_frem 14
#define ins_and 15
#define ins_or 16
#define ins_xor 17
#define ins_fneg 18
#define ins_icmp 19
#define ins_fcmp 20
#define ins_br 21
#define ins_define 22
#define ins_call 23
#define ins_ret 24
#define ins_label 25
#define ins_unreachable 26
#define ins_sitofp 27
#define ins_fptosi 28
#define ins_copy 29
#define ins_sext 30

/*instruction of asm*/
#define ins_asm_mv 0
#define ins_asm_la 1
#define ins_asm_li 2
#define ins_asm_lw 3
#define ins_asm_flw 4
#define ins_asm_ld 5
#define ins_asm_fld 6
#define ins_asm_sw 7
#define ins_asm_fsw 8
#define ins_asm_sd 9
#define ins_asm_fsd 10
#define ins_asm_fmv_x_w 11
#define ins_asm_fmv_w_x 12
#define ins_asm_add 13
#define ins_asm_addi 14
#define ins_asm_sub 15
#define ins_asm_subi 16
#define ins_asm_mul 17
#define ins_asm_div 18
#define ins_asm_rem 19
#define ins_asm_and 20
#define ins_asm_andi 21
#define ins_asm_or 22
#define ins_asm_ori 23
#define ins_asm_xor 24
#define ins_asm_xori 25
#define ins_asm_sll 26
#define ins_asm_srl 27
#define ins_asm_sra 28
#define ins_asm_slli 29
#define ins_asm_srli 30
#define ins_asm_srai 31
#define ins_asm_addw 32
#define ins_asm_addiw 33
#define ins_asm_subw 34
#define ins_asm_subiw 35
#define ins_asm_mulw 36
#define ins_asm_divw 37
#define ins_asm_remw 38
#define ins_asm_andw 39
#define ins_asm_andiw 40
#define ins_asm_orw 41
#define ins_asm_oriw 42
#define ins_asm_xorw 43
#define ins_asm_xoriw 44
#define ins_asm_sllw 45
#define ins_asm_srlw 46
#define ins_asm_sraw 47
#define ins_asm_slliw 48
#define ins_asm_srliw 49
#define ins_asm_sraiw 50
#define ins_asm_fadd_s 51
#define ins_asm_fsub_s 52
#define ins_asm_fmul_s 53
#define ins_asm_fdiv_s 54
#define ins_asm_frem_s 55
#define ins_asm_fneg_s 56
#define ins_asm_fcvt_s_w 57
#define ins_asm_fcvt_w_s 58
#define ins_asm_j 59
#define ins_asm_jal 60
#define ins_asm_jr 61
#define ins_asm_beq 62
#define ins_asm_bne 63
#define ins_asm_blt 64
#define ins_asm_bltu 65
#define ins_asm_ble 66
#define ins_asm_bleu 67
#define ins_asm_bgt 68
#define ins_asm_bgtu 69
#define ins_asm_bge 70
#define ins_asm_bgeu 71
#define ins_asm_feq_s 72
#define ins_asm_flt_s 73
#define ins_asm_fle_s 74
#define ins_asm_call 75
#define ins_asm_fcmpe_s 76
#define ins_asm_bgtz 77
#define ins_asm_label 78
/*cond of br*/
#define cond_eq 0
#define cond_ne 1
#define cond_ugt 2
#define cond_sgt 3
#define cond_ule 4
#define cond_sle 5
#define cond_uge 6
#define cond_sge 7
#define cond_ult 8
#define cond_slt 9
#define cond_oeq 10
#define cond_ogt 11
#define cond_oge 12
#define cond_olt 13
#define cond_ole 14
#define cond_une 15
/*number the registers*/
#define num_registers 32
/*type of registers*/
#define special 0
#define callee_saved 1
#define caller_saved 2
/*����*/
#define x0 0
#define x1 1
#define x2 2
#define x3 3
#define x4 4
#define x5 5
#define x6 6
#define x7 7
#define x8 8
#define x9 9
#define x10 10
#define x11 11
#define x12 12
#define x13 13
#define x14 14
#define x15 15
#define x16 16
#define x17 17
#define x18 18
#define x19 19
#define x20 20
#define x21 21
#define x22 22
#define x23 23
#define x24 24
#define x25 25
#define x26 26
#define x27 27
#define x28 28
#define x29 29
#define x30 30
#define x31 31

#define zero x0
#define ra x1
#define sp x2
#define gp x3
#define tp x4
#define t0 x5
#define t1 x6
#define t2 x7
#define s0 x8
#define s1 x9
#define a0 x10
#define a1 x11
#define a2 x12
#define a3 x13
#define a4 x14
#define a5 x15
#define a6 x16
#define a7 x17
#define s2 x18
#define s3 x19
#define s4 x20
#define s5 x21
#define s6 x22
#define s7 x23
#define s8 x24
#define s9 x25
#define s10 x26
#define s11 x27
#define t3 x28
#define t4 x29
#define t5 x30
#define t6 x31
/*������*/
#define f0 0
#define f1 1
#define f2 2
#define f3 3
#define f4 4
#define f5 5
#define f6 6
#define f7 7
#define f8 8
#define f9 9
#define f10 10
#define f11 11
#define f12 12
#define f13 13
#define f14 14
#define f15 15
#define f16 16
#define f17 17
#define f18 18
#define f19 19
#define f20 20
#define f21 21
#define f22 22
#define f23 23
#define f24 24
#define f25 25
#define f26 26
#define f27 27
#define f28 28
#define f29 29
#define f30 30
#define f31 31

#define ft0 f0
#define ft1 f1
#define ft2 f2
#define ft3 f3
#define ft4 f4
#define ft5 f5
#define ft6 f6
#define ft7 f7
#define fs0 f8
#define fs1 f9
#define fa0 f10
#define fa1 f11
#define fa2 f12
#define fa3 f13
#define fa4 f14
#define fa5 f15
#define fa6 f16
#define fa7 f17
#define fs2 f18
#define fs3 f19
#define fs4 f20
#define fs5 f21
#define fs6 f22
#define fs7 f23
#define fs8 f24
#define fs9 f25
#define fs10 f26
#define fs11 f27
#define ft8 f28
#define ft9 f29
#define ft10 f30
#define ft11 f31

#define fcsr 64

/*�ݴ�ռ�ƫ��*/
#define delta_Rd 0
#define delta_Rs1 8
#define delta_Rs2 16

/*��������*/
#define ret_i32 0
#define ret_float 1
#define ret_void 2
using namespace std;

/*register*/
struct Register_physical//����Ĵ���
{
	bool used,modified;//��Чλ,�޸�λ
	int num;//��Ӧ����Ĵ������
	vector<pair<int, int> >occupied;//��ռ�õ�����
	Register_physical()
	{
		occupied.clear();
	}
};

struct Register_virtual//����Ĵ���
{
	type_label name;//����
	int num,reg_phisical, is_splited_from;//���,���䵽������Ĵ������,���ĸ��Ĵ���splited����
	type_spliting_weight spliting_weight, prod;//prodָ�����ȶ����ڵ����ȼ�
	int type;//����
	bool used, is_splited, is_spilled, is_allocated;//��Чλ,����,�Ƿ��Ѿ���spilted����,�Ƿ��·ŵ��ڴ�,�Ƿ񱻷���Ĵ���
	vector < pair<int, int> >live_interval;//��Ծ����
	Register_virtual* next;
	Register_virtual()
	{
		live_interval.clear();
		reg_phisical = -1;
		is_splited = false;
		is_spilled = false;
		is_allocated = false;
		next = NULL;
	}
};

/*���ű�*/
struct variable_table
{
	int num, dim, num_reg, cnt;//���,ά��,��Ӧ�ļĴ������,����������
	int type;//����(i32,float,i64)
	type_variables name;//������
	vector<unsigned int>size, val;//ÿ��ά�ȵĴ�С,ÿ����λ��ֵ
	variable_table* next;
	variable_table()
	{
		dim = 0;
		cnt = 1;
		num_reg = -1;
		next = NULL;
	}
};

/*ָ���*/
struct instruction
{
	int num;//ָ����
	int op;//ָ������
	int Rd, Rs1, Rs2;//������ţ�Ŀ�ģ�Դ1��Դ2
	bool fRd, fRs1, fRs2;//�������ͣ��ֲ���ȫ��
	int tRd, tRs1, tRs2;//�������ͣ�i32��i64��������
	bool fimm, fimm1, fimm2;//�Ƿ���ƫ����
	unsigned int imm, imm1, imm2;//ƫ����������GEPָ��,storeָ��,����ָ��
	/*���xcmp���*/
	int cond;
	/*���br���*/
	bool branch_flag;//�Ƿ�Ϊ��������ת
	int L1, L2;//��ת��Ŷ�Ӧ���
	/*���call,ret���*/
	int tot_formal,type_ret;//�β���������������
	type_label name;//����������
	int num_bb;//����bb���
	bool all_imm;//GEPָ���Ƿ�ȫΪ������
	int size;//GEPָ���Ӧ��ַ��С
	vector<bool>formal_type;//�洢�βε�����
	vector<bool>formal_is_imm;//�β��Ƿ���������
	vector<unsigned int>formal_num;//�洢�βεı��,Ϊ�������Ļ����Ӧ��������
	vector<int>gep_size;//GEPָ��ÿ��ά�ȵĴ�С
	instruction* next;
	instruction()
	{
		Rd = Rs1 = Rs2 = -1;
		fimm = fimm1 = fimm2 = 0;
		imm = imm1 = imm2 = 0;
		branch_flag = 0;
		tot_formal = 0;
		all_imm = true;
		next = NULL;
	}
};

/*���ָ��*/
struct asm_instruction
{
	int num;//ָ����
	int op;//ָ������
	int Rd, Rs1, Rs2;//������ţ�Ŀ�ģ�Դ1��Դ2
	int imm;//ƫ����
	int label;//��ת��š���������Ӧ���
	asm_instruction* next, * prev;
	asm_instruction()
	{
		Rd = Rs1 = Rs2 = -1;
		imm = 0;
		next = NULL; prev = NULL;
	}
};

/*basic block*/
struct basic_block
{
	int num;//bb���
	int l, r;//��ָֹ����
	bool is_loop;//�Ƿ�Ϊѭ��ָ��
	instruction* ins_head;//��ʼָ��ָ��
	vector<basic_block*>edge;//����
	basic_block* next;
	basic_block()
	{
		ins_head = new instruction;
		next = NULL;
		is_loop = false;
	}
};

/*functions*/
struct functions
{
	/*�������, ָ����, bb��, ʵ������, �β�����, ���õ��β��������ֵ, ��������, �����ջ�ռ��С,
	����ֵ����,��ʼָ����*/
	int num, cnt_ins, cnt_bb, total_actual, total_formal, max_formal, tot_arg, size, type, ins_start;
	type_label name;//��������
	vector<int>args;//��������
	instruction* ins_head, * ins_tail;//ָ��ͷβָ��
	asm_instruction* asm_ins_head, * asm_ins_tail;//���ָ��ͷβָ��
	basic_block* bb_head, * bb_tail;//bbͷβָ��
	variable_table* local_head, * local_tail;//�ֲ�������
	map<type_variables, int>map_local;//�ֲ�����������ŵ�ӳ��
	map<int, int>map_physical_register_local;//����Ĵ�����ŵ�������ŵ�ӳ��
	map<int, int>imm_local;//������ŵ���ַƫ������ӳ��
	Register_virtual* reg_head, * reg_tail;//����Ĵ���
	map<type_label, int>map_local_register;//����Ĵ���������ŵ�ӳ��
	map<int, Register_virtual*>map_local_register_position;//����Ĵ�����ŵ�ָ���ӳ��
	/*��¼ʹ���˵ļĴ���,��ǰ����Ӧ�ñ����caller_saved��callee_saved�Ĵ���*/
	vector<bool>used_physical_reg, caller_saved_reg, callee_saved_reg;
	functions* next;
	functions()
	{
		cnt_ins = 0, cnt_bb = 0; total_actual = 0; total_formal = 0; max_formal = 0; tot_arg = 0; size = 0;
		ins_head = new instruction;
		ins_tail = ins_head;
		bb_head = new basic_block;
		bb_tail = bb_head;
		local_head = new variable_table;
		local_tail = local_head;
		reg_head = new Register_virtual;
		reg_tail = reg_head;
		asm_ins_head = new asm_instruction;
		asm_ins_tail = asm_ins_head;
		used_physical_reg.resize(num_registers << 1);
		caller_saved_reg.resize(num_registers << 1);
		callee_saved_reg.resize(num_registers << 1);
		next = NULL;
	}
};

/*definitions*/
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
extern map<type_variables, int>map_global;//ȫ�ֱ���������ŵ�ӳ��
extern map<int, variable_table*>map_variable_position;//������ŵ�ָ���ӳ��
extern map<int, int>map_register_local;//�Ĵ�����ŵ�������ŵ�ӳ��
extern map<int, int>map_register_local_alloca;//alloca����ڼĴ�����ŵ�������ŵ�ӳ��

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

extern map<type_label, int>label_num;//label������ŵ�ӳ��
extern map<int, int>label_ins;//label��ָ���ӳ��
extern vector<basic_block*>label_bb;//label��Ŷ�Ӧ��bb���
extern int tot_label;//label����

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern map<int, functions*>map_function_position;//���ݺ�����Ų��Һ���ָ��
extern int tot_functions;//�ܵĺ�����

/*function names*/
unsigned int floatToBinary(float num);
float BinaryToFloat(unsigned int num);
void read(int option, std::string line, functions* num, bool in_func);
functions* new_function(std::string line);
void end_function(functions* now_function);
int check_label(std::string s);
std::string get_label(std::string s);
void new_label(int op, type_label label, functions* num_func);
std::string get_new_line(std::string line);
void get_basic_block(functions* now_func);
void get_live_interval(functions* now_func);
void resize_live_interval(Register_virtual* now_reg);//����Ĵ����Ļ�Ծ����(���ϲ�)
void check_live_interval(Register_virtual* now_reg);
void register_allocate(functions* now_func);
void get_asm();//�õ�������
void peephole(functions* now_func);//�����Ż�
void print_asm(functions* now_func);//���������
void get_callee_saved_reg();//�õ�callee_saved�Ĵ���,����ռ�
void allocate_physical_reg(functions* now_func);//Ϊ����Ĵ�������ջ�ռ�
std::string get_function_name(std::string line);