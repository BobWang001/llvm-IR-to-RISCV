#include "riscv.h"

vector<basic_block*>bb_stack;//将经过的bb放入栈中,vector模拟
set<int>bb_in_stack;//记录在栈内的bb
map<int, int>reg_definied;//记录虚拟寄存器被赋值的位置
map<int, int>reg_used;//记录虚拟寄存器被使用的位置
vector<int>reg_num;//记录定义过的局部虚拟寄存器
extern set<int>ins_definied;//会定义虚拟寄存器的指令(除call)
extern set<int>ins_used;//会使用虚拟寄存器的指令

bool cmp(pair<int, int>a, pair<int, int>b)
{
	if (a.first == b.first)
		return a.second < b.second;
	else return a.first < b.first;
}

void clear()
{
	bb_stack.clear(); bb_stack.shrink_to_fit();
	bb_in_stack.clear();
	reg_definied.clear();
	reg_used.clear();
	reg_num.clear(); reg_num.shrink_to_fit();
}

bool check_definied(instruction* ins_head)
{
	if (ins_definied.find(ins_head->op) != ins_definied.end())
		return true;
	if (ins_head->op == ins_call && ins_head->type_ret != 2)
		return true;
	return false;
}

void reg_write(instruction* ins_head, int reg_num, int val)
{
	if (reg_used.count(reg_num) == 0)
		reg_used[reg_num] = val;
	else reg_used[reg_num] = max(reg_used[reg_num], val);
}

void get_used(instruction* ins_head)
{
	if (ins_used.find(ins_head->op) != ins_used.end())//Rs1绝对为寄存器
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
	if (ins_head->op == ins_store)//store指令用到了Rd
	{
		reg_write(ins_head, ins_head->Rd, ins_head->num);
		if(ins_head->fimm==0)
			reg_write(ins_head, ins_head->Rs1, ins_head->num);
	}
	if (ins_head->op >= ins_add && ins_head->op <= ins_fcmp)//需要判断立即数
	{
		if (ins_head->fimm1 == 0)
			reg_write(ins_head, ins_head->Rs1, ins_head->num);
		if (ins_head->fimm2 == 0 && ins_head->op != ins_fneg)
			reg_write(ins_head, ins_head->Rs2, ins_head->num);
	}
	if (ins_head->op == ins_br && ins_head->branch_flag)//需要判断条件跳转
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
	if (ins_head->op == ins_call)//遍历参数列表
	{
		for (auto it : ins_head->formal_num)
			reg_write(ins_head, it, ins_head->num);
	}
	if (ins_head->op == ins_ret && ins_head->type_ret != 2)//ret指令需要判断是否为void
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
}

/*在虚拟寄存器内插入活跃区间*/
void add_live_interval(Register_virtual* reg_num, int l, int r)
{
	for (auto it : bb_stack)
	{
		int L = max(l, it->l), R = min(r, it->r);
		if (L > R)
			continue;
		reg_num->live_interval.push_back({ L,R });
	}
}

void dfs(functions* now_func, basic_block* now_bb)
{
	if (now_bb->next == NULL)//结尾bb
		return;
	if (bb_in_stack.find(now_bb->num) != bb_in_stack.end())//出现环
	{
		bool find_loop = false;
		for (auto it : bb_stack)
		{
			if (it->num == now_bb->num)
				find_loop = true;
			if (find_loop)
				it->is_loop = true;//标记循环内的bb
		}
		return;
	}
	bb_stack.push_back(now_bb);
	bb_in_stack.insert(now_bb->num);
	instruction* ins_head = now_bb->ins_head;
	while (ins_head != NULL && ins_head->num <= now_bb->r)
	{
		if (check_definied(ins_head))//有寄存器被定义
		{
			reg_definied[ins_head->Rd] = ins_head->num;//记录被定义的位置
			reg_num.push_back(ins_head->Rd);
		}
		get_used(ins_head);//将使用过的寄存器记录
		ins_head = ins_head->next;
	}
	for (auto it : reg_used)
	{
		int l = reg_definied[it.first], r = it.second;
		if (map_global_register_position.count(it.first) != 0)
			add_live_interval(map_global_register_position[it.first], l, r);
		else
			add_live_interval(now_func->map_local_register_position[it.first], l, r);//将区间加入寄存器活跃区间
	}
	reg_used.clear();
	for (auto it : now_bb->edge)
		dfs(now_func, it);
	bb_stack.pop_back();
	bb_in_stack.erase(now_bb->num);
}

void resize_live_interval(Register_virtual* now_reg)
{
	vector<pair<int, int> >lst_live_interval;
	lst_live_interval.clear();
	for (auto it : now_reg->live_interval)
		lst_live_interval.push_back({ it.first,it.second });
	sort(lst_live_interval.begin(), lst_live_interval.end(), cmp);//先将区间排列
	now_reg->live_interval.clear();
	now_reg->live_interval.shrink_to_fit();
	int L = -1, R = -1;
	for (auto it : lst_live_interval)
	{
		int l = it.first, r = it.second;
		if (L == -1)
		{
			L = l; R = r;
		}
		else
		{
			if (l > R + 1)//划分新区间(集合求并集)
			{
				now_reg->live_interval.push_back({ L,R });
				L = l; R = r;
			}
			else R = max(R, r);
		}
	}
	if (L != -1)
		now_reg->live_interval.push_back({ L,R });
	lst_live_interval.clear();
	lst_live_interval.shrink_to_fit();
}

void check_live_interval(Register_virtual* now_reg)//调试信息
{
	printf(";reg_name=%s reg_number=%d live_intervals: ", now_reg->name.c_str(), now_reg->num);
	for (auto it : now_reg->live_interval)
		printf("[%d,%d] ", it.first, it.second);
	printf("\n\n");
}

void get_live_interval(functions* now_func)
{
	basic_block* bb_head = now_func->bb_head;
	bb_head = bb_head->next;
	if (bb_head->next == now_func->bb_tail)
		return;
	bb_head = bb_head->next;
	for (auto it : map_global)//先插入全局变量被赋值的位置
		reg_definied[it.second] = bb_head->ins_head->num;
	dfs(now_func, bb_head);
	for (auto it : reg_num)
	{
		Register_virtual* now_reg = now_func->map_local_register_position[it];
		resize_live_interval(now_reg);//整理寄存器的活跃区间(集合并)
		//check_live_interval(now_reg);
	}
	
	clear();
}