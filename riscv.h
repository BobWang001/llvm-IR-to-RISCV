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
#include <fstream>
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
	bool uesd,modified;//修改位
	type_variables name;//对应变量名
}reg[num_registers];

/*符号表*/
struct variable_table
{
	int num,num_reg;//编号,对应的寄存器编号
	bool type;//类型(int,float)
	type_variables name;//变量名
	variable_table* next;
}*local,*global;//局部,全局
int total_global=0;//存储全局变量的数量
map<type_variables, int>rev_local, rev_global;

/*指令翻译*/
struct instruction
{
	int num;//指令编号
	int op;//指令类型
	int Rd,Rs1,Rs2;//变量编号：目的，源1，源2
	bool fRd,fRs1,fRs2;//变量类型：局部，全局
	type_label L1, L2;//跳转标号
	int imm;//偏移量，用于GEP指令
	instruction* next;
};

/*basic block*/
struct basic_block
{
	int num;//bb编号
	int l, r;//起止指令编号
	vector<int>edge;//连边
	basic_block* next;
};