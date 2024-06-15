#pragma once
#include <iostream>
#include <cstdio>
#include <cmath>
#include <ctime>
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
#define ins_fneg 15
#define ins_icmp 16
#define ins_fcmp 17
#define ins_br 18
#define ins_define 19
#define ins_call 20
#define ins_ret 21
#define ins_label 22
#define ins_unreachable 23
#define ins_sitofp 24
#define ins_fptosi 25

/*number the registers*/
#define num_registers 32
/*整型*/
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
/*浮点型*/
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
using namespace std;

/*register*/
struct Register_physical//物理寄存器
{
	bool used,modified;//有效位,修改位
	int num;//对应虚拟寄存器编号
	vector<pair<int, int> >occupied;//被占用的区间
	Register_physical()
	{
		occupied.clear();
	}
};

struct Register_virtual//虚拟寄存器
{
	type_label name;//名称
	int num,reg_phisical, is_splited_from;//编号,分配到的物理寄存器编号,从哪个寄存器splited过来
	type_spliting_weight spliting_weight, prod;//prod指在优先队列内的优先级
	int type;//类型
	bool used, is_splited, is_spilled;//有效位,类型,是否已经被spilted过了,是否下放到内存
	vector < pair<int, int> >live_interval;//活跃区间
	Register_virtual* next;
	Register_virtual()
	{
		live_interval.clear();
		reg_phisical = -1;
		is_splited = false;
		is_spilled = false;
		next = NULL;
	}
};

/*符号表*/
struct variable_table
{
	int num, dim, num_reg, cnt;//编号,维度,对应的寄存器编号,变量的数量
	int type;//类型(i32,float,i64)
	type_variables name;//变量名
	vector<unsigned int>size, val;//每个维度的大小,每个单位的值
	variable_table* next;
	variable_table()
	{
		dim = 0;
		cnt = 1;
		num_reg = -1;
		next = NULL;
	}
};

/*指令翻译*/
struct instruction
{
	int num;//指令编号
	int op;//指令类型
	int Rd, Rs1, Rs2;//变量编号：目的，源1，源2
	bool fRd, fRs1, fRs2;//变量类型：局部，全局
	int tRd, tRs1, tRs2;//变量类型：i32，i64，浮点型
	bool fimm, fimm1, fimm2;//是否有偏移量
	unsigned int imm, imm1, imm2;//偏移量，用于GEP指令,store指令,运算指令
	/*针对xcmp语句*/
	int cond;
	/*针对br语句*/
	bool branch_flag;//是否为无条件跳转
	int L1, L2;//跳转标号对应编号
	/*针对call,ret语句*/
	int tot_formal,type_ret;//形参数量，返回类型
	type_label name;//被调函数名
	int num_bb;//所在bb编号
	vector<bool>formal_type;//存储形参的类型
	vector<int>formal_num;//存储形参的编号
	instruction* next;
	instruction()
	{
		Rd = Rs1 = Rs2 = -1;
		fimm = fimm1 = fimm2 = 0;
		branch_flag = 0;
		tot_formal = 0;
		next = NULL;
	}
};

/*basic block*/
struct basic_block
{
	int num;//bb编号
	int l, r;//起止指令编号
	bool is_loop;//是否为循环指令
	instruction* ins_head;//起始指令指针
	vector<basic_block*>edge;//连边
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
	//函数编号,指令数,bb数,实参数量,形参数量,调用的形参数量最大值,参数数量,所需的栈空间大小,返回值类型
	int num, cnt_ins, cnt_bb, total_actual, total_formal, max_formal, tot_arg, size, type;
	type_label name;//函数名称
	vector<bool>args;//参数类型
	instruction* ins_head, * ins_tail;//指令头尾指针
	basic_block* bb_head, * bb_tail;//bb头尾指针
	variable_table* local_head, * local_tail;//局部变量表
	map<type_variables, int>map_local;//变量名到编号的映射
	map<int, int>imm_local;//变量编号到地址偏移量的映射
	Register_virtual* reg_head, * reg_tail;//虚拟寄存器
	map<type_label, int>map_local_register;//虚拟寄存器名到编号的映射
	map<int, Register_virtual*>map_local_register_position;//虚拟寄存器编号到指针的映射
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
		next = NULL;
	}
};

/*definitions*/
extern Register_physical reg[2][num_registers];
extern map<type_label, int>map_global_register;//全局寄存器名到编号的映射
extern map<int, Register_virtual*>map_global_register_position;//全局寄存器编号到指针的映射
extern map<int, Register_virtual*>map_local_register_position;//局部虚拟寄存器编号到指针的映射
extern map<int, std::string>map_physical_reg_name[2];//物理寄存器编号到名称的映射
extern vector<bool>physical_reg_usable[2];//物理寄存器能否使用
extern vector<int>physical_reg_order[2];//考虑分配物理寄存器的顺序
extern int total_register;

extern variable_table* global, * global_tail;
extern int total_global;//存储变量的数量
extern map<type_variables, int>map_global;//变量名到编号的映射
extern map<int, int>map_register_local;//寄存器编号到变量编号的映射

extern instruction* start;//属于全局的指令
extern map<std::string, int>ins_num, cond_num;
extern int tot_instructions;//总的指令数
extern set<int>ins_definied;//会定义虚拟寄存器的指令(除call)
extern set<int>ins_used;//会使用虚拟寄存器的指令
extern set<int>ins_valuate;//会对Rd赋值的指令
extern map<int, instruction*>map_instruction_position;//指令编号到指针的映射
extern map<int, pair<int, int> >map_ins_copy;//记录copy指令的位置

extern map<type_label, int>label_num;//label名到编号的映射
extern map<int, int>label_ins;//label到指令的映射
extern vector<basic_block*>label_bb;//label编号对应的bb编号
extern int tot_label;//label数量

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern int tot_functions;//总的函数数

/*function names*/
unsigned int floatToBinary(float num);
void read(int option, std::string line, functions* num, bool in_func);
functions* new_function(std::string line);
void end_function(functions* now_function);
int check_label(std::string s);
std::string get_label(std::string s);
void new_label(int op, type_label label, functions* num_func);
std::string get_new_line(std::string line);
void get_basic_block(functions* now_func);
void get_live_interval(functions* now_func);
void resize_live_interval(Register_virtual* now_reg);//整理寄存器的活跃区间(集合并)
void check_live_interval(Register_virtual* now_reg);
void register_allocate(functions* now_func);