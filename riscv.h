#pragma once
#include <iostream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
/*type of variables*/
#define type_variables std::string
#define type_registers int
#define type_label std::string
/*number the registers*/
#define num_registers 64
#define zero 0
#define ra 1
#define sp 2
#define t0 3
#define t1 4
#define t2 5
#define t3 6
#define t4 7
#define t5 8
#define t6 9
#define s0 10
#define s1 11
#define s2 12
#define s3 13
#define s4 14
#define s5 15
#define s6 16
#define s7 17
#define s8 18
#define s9 19
#define s10 20
#define s11 21
#define a0 22
#define a1 23
#define a2 24
#define a3 25
#define a4 26
#define a5 27
#define a6 28
#define a7 29
#define ft0 30
#define ft1 31
#define ft2 32
#define ft3 33
#define ft4 34
#define ft5 35
#define ft6 36
#define ft7 37
#define fs0 38
#define fs1 39
#define fa0 40
#define fa1 41
#define fa2 42
#define fa3 43
#define fa4 44
#define fa5 45
#define fa6 46
#define fa7 47
#define fs2 48
#define fs3 49
#define fs4 50
#define fs5 51
#define fs6 52
#define fs7 53
#define fs8 54
#define fs9 55
#define fs10 56
#define fs11 57
#define ft8 58
#define ft9 59
#define ft10 60
#define ft11 61
using namespace std;

/*register*/
struct Register
{
	bool uesd,modified;//有效位,修改位
	type_variables name;//对应变量名
};

/*符号表*/
struct variable_table
{
	int num, dim, num_reg, cnt;//编号,维度,对应的寄存器编号,变量的数量
	bool type;//类型(int,float)
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
};//全局

/*指令翻译*/
struct instruction
{
	int num;//指令编号
	int op;//指令类型
	int Rd, Rs1, Rs2;//变量编号：目的，源1，源2
	bool fRd, fRs1, fRs2;//变量类型：局部，全局
	bool tRd, tRs1, tRs2;//变量类型：整型，浮点型
	bool fimm, fimm1, fimm2;//是否有偏移量
	unsigned int imm, imm1, imm2;//偏移量，用于GEP指令,store指令,运算指令
	/*针对xcmp语句*/
	int cond;
	/*针对br语句*/
	type_label L1, L2;//跳转标号
	/*针对call,ret语句*/
	int tot_formal,type_ret;//形参数量，返回类型
	type_label name;//被调函数名
	vector<bool>formal_type;//存储形参的类型
	vector<int>formal_num;//存储形参的编号
	instruction* next;
	instruction()
	{
		tot_formal = 0;
		next = NULL;
	}
};

/*basic block*/
struct basic_block
{
	int num;//bb编号
	int l, r;//起止指令编号
	vector<int>edge;//连边
	basic_block* next;
	basic_block()
	{
		next = NULL;
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
	functions* next;
	functions()
	{
		cnt_ins = 0, cnt_bb = 0; total_actual = 0; total_formal = 0; tot_arg = 0; size = 0;
		ins_head = new instruction;
		ins_tail = ins_head;
		bb_head = new basic_block;
		bb_tail = bb_head;
		local_head = new variable_table;
		local_tail = local_head;
		next = NULL;
	}
};

/*definitions*/
extern Register reg[num_registers];

extern variable_table* global, * global_tail;
extern int total_global;//存储全局变量的数量
extern map<type_variables, int>map_global;//变量名到编号的映射

extern instruction* start;//属于全局的指令
extern map<std::string, int>ins_num, cond_num;
extern int tot_instructions;//总的指令数

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern int tot_functions;//总的函数数

/*function names*/
unsigned int floatToBinary(float num);
void read(int option, std::string line, functions* num, bool in_func);
void new_variable(std::string line, variable_table* tail);
void new_variable_type(variable_table* new_global, std::string word);
void new_variable_value(variable_table* new_global, std::string word);
functions* new_function(std::string line);
void end_function(functions* now_function);
void get_basic_block(functions* now_function);
