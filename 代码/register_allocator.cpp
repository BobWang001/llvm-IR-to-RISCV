#include "riscv.h"

extern Register_physical reg[2][num_registers];
extern map<int, Register_virtual*>map_global_register_position;//全局寄存器编号到指针的映射
extern map<int, Register_virtual*>map_local_register_position;//局部虚拟寄存器编号到指针的映射
extern map<int, int>map_register_local;//寄存器编号到变量编号的映射
extern set<int>ins_definied;//会定义虚拟寄存器的指令(除call)
extern set<int>ins_used;//会使用虚拟寄存器的指令
extern vector<bool>physical_reg_usable[2];//物理寄存器能否使用
extern vector<int>physical_reg_order[2];//考虑分配物理寄存器的顺序
extern vector<int>physical_reg_saved[2];//special,caller_saved,callee_saved
extern map<int, instruction*>map_instruction_position;//指令编号到指针的映射
extern set<int>ins_valuate;//会对Rd赋值的指令
extern map<int, pair<int, int> >map_ins_copy;//记录copy指令的位置
priority_queue<pair<type_spliting_weight, type_registers> >allocate_queue;
struct cmp_set
{
	bool operator()(int x, int y) const
	{
		Register_virtual* reg_x = map_local_register_position[x];
		Register_virtual* reg_y = map_local_register_position[y];
		return (reg_x->spliting_weight < reg_y->spliting_weight) ? true : false;
	}
};
set<int, cmp_set>allocated_reg;//存储已经分配好的寄存器

void allocate_clear()
{
	for (int i = 0; i < num_registers; ++i)
	{
		reg[0][i].occupied.clear();
		reg[1][i].occupied.clear();
	}
	while (allocate_queue.size())
		allocate_queue.pop();
	allocated_reg.clear();
}

bool cmp_reg(pair<int,int>a, pair<int,int>b)
{
	if (a.first == b.first)
		return a.second < b.second;
	else return a.first < b.first;
}

void check_register_allocate(functions* now_func)
{
	Register_virtual* reg_head = now_func->reg_head;
	while (reg_head->next != NULL)
	{
		reg_head = reg_head->next;
		printf(";reg_name=%s reg_number=%d live_intervals: ", reg_head->name.c_str(), reg_head->num);
		for (auto it : reg_head->live_interval)
			printf("[%d,%d] ", it.first, it.second);
		printf("\n;spliting_weight=%lld prod_value=%lld ",
			reg_head->spliting_weight, reg_head->prod);
		if (reg_head->is_splited)
			printf("is_splited_from=%d ", reg_head->is_splited_from);
		if (reg_head->is_spilled)
			printf("is_spilled\n\n");
		else
			printf("physical_reg=%d\n\n", reg_head->reg_phisical);
	}
	printf("\n");
}

type_spliting_weight count_use_reg(instruction* ins_head, int reg_num)
{
	type_spliting_weight count_reg = 0;
	if (ins_head->Rd == reg_num)
		count_reg++;
	if (ins_head->Rs1 == reg_num)
		count_reg++;
	if (ins_head->Rs2 == reg_num)
		count_reg++;
	return count_reg;
}

type_spliting_weight calc_spliting_weight(Register_virtual* now_reg,int reg_num, functions* now_func)
{
	type_spliting_weight value = 0, history_max = -1;
	basic_block* bb_head = now_func->bb_head;
	bb_head = bb_head->next;
	if (bb_head->next == NULL)
		return value;
	for (auto it : now_reg->live_interval)
	{
		int l = it.first, r = it.second;
		/*找到使用了该寄存器的bb*/
		while (bb_head->next != NULL && bb_head->r < l)
			bb_head = bb_head->next;
		if (bb_head->next == NULL)
			return value;
		while (bb_head->next != NULL && bb_head->l <= r)
		{
			/*遍历指令*/
			instruction* ins_head = bb_head->ins_head;
			type_spliting_weight delta = 0;
			while (ins_head != NULL)
			{
				delta += count_use_reg(ins_head, reg_num);//使用当前寄存器的次数
				ins_head = ins_head->next;
			}
			if (bb_head->is_loop)
				delta *= 3;
			value += delta;
			history_max = max(history_max, value);
			bb_head = bb_head->next;
		}
	}
	if (value < history_max)//判断溢出
		value = LLONG_MAX;//设为最大值
	return value;
}

type_spliting_weight calc_prod(Register_virtual* now_reg, functions* now_func)
{
	type_spliting_weight value = 0;
	for (auto it : now_reg->live_interval)
	{
		int l = it.first, r = it.second;
		value += r - l + 1;
	}
	return value;
}

bool check_ins(int num_ins, int op, int num_physical_reg, functions* now_func,
	int num_virtual_reg = 0)
{
	instruction* now_ins = map_instruction_position[num_ins];
	/*判断是否为Rd赋值*/
	if (ins_valuate.find(now_ins->op) != ins_valuate.end() || (now_ins->op == ins_call && now_ins->type_ret != 2))
	{
		if (op)
			return (num_virtual_reg == now_ins->Rd) ? true : false;
		else
		{
			Register_virtual* reg_Rd;
			//if (map_global_register_position.count(now_ins->Rd) != 0)
			//	reg_Rd = map_global_register_position[now_ins->Rd];
			//else
				reg_Rd = map_local_register_position[now_ins->Rd];
			return (reg_Rd->reg_phisical == num_physical_reg) ? true : false;
		}
	}
	return false;
}

bool intersect(Register_virtual* now_reg, vector<pair<int, int> >reg_interval,
	int num_physical_reg, functions* now_func)
{
	vector<pair<int, int> >::iterator reg_it = reg_interval.begin();
	int l2, r2;
	if (reg_it == reg_interval.end())//占用区间为空
		return false;
	l2 = (*reg_it).first; r2 = (*reg_it).second;
	for (auto it : now_reg->live_interval)
	{
		int l1 = it.first, r1 = it.second;
		while (reg_it != reg_interval.end() && r2 < l1)
		{
			reg_it++;
			if (reg_it != reg_interval.end())
			{
				l2 = (*reg_it).first;
				r2 = (*reg_it).second;
			}
		}
		if (reg_it == reg_interval.end())
			break;
		if (r2 > l1 || l2 < r1)
			return true;
		if (r2 == l1)
			if (!check_ins(l1, 0, num_physical_reg, now_func, now_reg->num))//若不为now_reg赋值，则不可以分配
				return true;
		if (l2 == r1)
			if (!check_ins(l2, 1, num_physical_reg, now_func))//若未对物理寄存器赋值,则不可以分配
				return true;
	}
	return false;
}

/*分配物理寄存器*/
void allocate(Register_virtual* now_reg, Register_physical& physical_reg, int num_reg)
{
	now_reg->is_allocated = true;
	now_reg->reg_phisical = num_reg;
	for (auto it : now_reg->live_interval)
		physical_reg.occupied.push_back({ it.first,it.second });
	sort(physical_reg.occupied.begin(), physical_reg.occupied.end(), cmp_reg);
	allocated_reg.insert(now_reg->num);
}

/*尝试分配寄存器*/
bool try_assign(Register_virtual* now_reg, functions* now_func)
{
	int op = (now_reg->type == float32) ? 1 : 0;
	for (auto it : physical_reg_order[op])
	{
		if (physical_reg_usable[op][it] == false)
			continue;
		if (intersect(now_reg, reg[op][it].occupied, op * num_registers + it, now_func))
			continue;
		/*可以分配寄存器*/
		allocate(now_reg, reg[op][it], op * num_registers + it);
		return true;
	}
	return false;
}

vector<pair<int, int> >get_new_interval(Register_virtual* now_reg,
	vector<pair<int, int> >reg_interval)
{
	vector<pair<int, int> >ret_interval;
	ret_interval.clear();
	vector<pair<int, int> >::iterator reg_it = reg_interval.begin();
	if (reg_it == reg_interval.end())
		return reg_interval;
	int l2 = (*reg_it).first, r2 = (*reg_it).second;
	for (auto it : now_reg->live_interval)
	{
		int l1 = it.first, r1 = it.second;
		while (l2 < l1)
		{
			ret_interval.push_back({ l2,r2 });
			reg_it++;
			l2 = (*reg_it).first, r2 = (*reg_it).second;
		}
		//l1=r1,l2=r2
		reg_it++;
		if (reg_it == reg_interval.end())
			break;
		l2 = (*reg_it).first, r2 = (*reg_it).second;
	}
	return ret_interval;
}

/*尝试弹出一个spliting weight较小的寄存器*/
bool try_evction(Register_virtual* now_reg, functions* now_func)
{
	int op = (now_reg->type == float32) ? 1 : 0;
	for (auto it : allocated_reg)
	{
		Register_virtual* it_reg = map_local_register_position[it];
		if (it_reg->spliting_weight >= now_reg->spliting_weight)
			return false;
		if (it_reg->type != now_reg->type)
			continue;
		int num_physical_reg = it_reg->reg_phisical;
		vector<pair<int, int> >new_interval = get_new_interval(it_reg, reg[num_physical_reg / num_registers][num_physical_reg % num_registers].occupied);
		if (!intersect(now_reg, new_interval, num_physical_reg, now_func))
		{
			allocate(now_reg, reg[num_physical_reg / num_registers][num_physical_reg % num_registers], it_reg->reg_phisical);
			allocate_queue.push({ it_reg->prod,it_reg->num });
			allocated_reg.erase(it);
			return true;
		}
	}
	return false;
}

/*将一个虚拟寄存器拆分为两个*/
bool get_splited_reg(Register_virtual* now_reg, vector<pair<int, int> >reg_interval,
	int num_physical_reg, functions* now_func, Register_virtual* ret_reg_first,
	Register_virtual* ret_reg_second)
{
	vector<pair<int, int> >::iterator reg_it = reg_interval.begin();
	if (reg_it == reg_interval.end())
		return false;
	int l2 = (*reg_it).first, r2 = (*reg_it).second;
	for (auto it : now_reg->live_interval)
	{
		int l1 = it.first, r1 = it.second;
		while (reg_it != reg_interval.end() && r2 < l1)
		{
			reg_it++;
			if (reg_it != reg_interval.end())
			{
				l2 = (*reg_it).first;
				r2 = (*reg_it).second;
			}
		}
		if (reg_it == reg_interval.end())
			ret_reg_first->live_interval.push_back({ l1,r1 });
		else
		{
			int l = max(l1, l2), r = min(r1, r2);
			if (l > r)//交集为空
				ret_reg_first->live_interval.push_back({ l1,r1 });
			else
			{
				if (l == r && l == r1)
				{
					if (check_ins(l, 0, num_physical_reg, now_func, now_reg->num))
						l++;
				}
				if (l == r && l == l1)
				{
					if (check_ins(l, 1, num_physical_reg, now_func))
						r--;
				}
				if (l <= r)
				ret_reg_second->live_interval.push_back({ l,r });//区间交给第二个寄存器
				if (l1 <= l - 1)
					ret_reg_first->live_interval.push_back({ l1,l - 1 });
				if (r + 1 <= r1)
					ret_reg_first->live_interval.push_back({ r + 1,r1 });
			}
		}
	}
	if (ret_reg_first->live_interval.size() == 0)
		return false;
	if (ret_reg_second->live_interval.size() == 0)
		return false;
	ret_reg_first->prod = calc_prod(ret_reg_first, now_func);
	ret_reg_first->spliting_weight = calc_spliting_weight(ret_reg_first, now_reg->num, now_func);
	ret_reg_second->prod = calc_prod(ret_reg_second, now_func);
	ret_reg_second->spliting_weight = calc_spliting_weight(ret_reg_second, now_reg->num, now_func);
	return true;
}

/*加入到寄存器列表*/
void add_new_register(Register_virtual* new_register, Register_virtual* now_register,
	functions* now_func, int append)
{
	new_register->name = now_register->name + ((append) ? "b" : "a");
	new_register->num = ++total_register;
	new_register->type = now_register->type;
	new_register->is_splited = true;
	new_register->is_splited_from = now_register->num;
	now_func->map_local_register_position[new_register->num] = new_register;
	map_local_register_position[new_register->num] = new_register;
	if (map_register_local_alloca.count(now_register->num) != 0)
		map_register_local_alloca[new_register->num] = map_register_local_alloca[now_register->num];
	now_func->reg_tail->next = new_register;
	now_func->reg_tail = new_register;
	allocate_queue.push({ new_register->prod,new_register->num });
}

void reg_copy(Register_virtual* now_register, Register_virtual* new_register)
{
	now_register->is_allocated = new_register->is_allocated;
	now_register->is_spilled = new_register->is_spilled;
	now_register->is_splited = new_register->is_splited;
	now_register->is_splited_from = new_register->is_splited_from;
	now_register->live_interval = new_register->live_interval;
	now_register->name = new_register->name;
	now_register->num = new_register->num;
	now_register->prod = new_register->prod;
	now_register->reg_phisical = new_register->reg_phisical;
	now_register->spliting_weight = new_register->spliting_weight;
	now_register->type = new_register->type;
	now_register->used = new_register->used;
}

/*替换指令相关的寄存器*/
void change_instruction(Register_virtual* now_register, Register_virtual* new_register,
	functions* now_func)
{
	basic_block* bb_head = now_func->bb_head;
	bb_head = bb_head->next;
	if (bb_head->next == now_func->bb_tail)
		return;
	bb_head = bb_head->next;
	for (auto it : new_register->live_interval)
	{
		int l = it.first, r = it.second;
		while (bb_head != now_func->bb_tail && bb_head->r < l)
			bb_head = bb_head->next;
		if (bb_head == now_func->bb_tail)
			return;
		instruction* ins_head = bb_head->ins_head;
		while (ins_head->next != NULL)
		{
			ins_head = ins_head->next;
			if (ins_head->num >= l && ins_head->num <= r)
			{
				if (ins_head->Rd == now_register->num)
					ins_head->Rd = new_register->num;
				if (ins_head->Rs1 == now_register->num)
					ins_head->Rs1 = new_register->num;
				if (ins_head->Rs2 == now_register->num)
					ins_head->Rs2 = new_register->num;
			}
			if (ins_head->num > r)
				break;
			if (ins_head->op == ins_call)
			{
				for (int i = 0; i < ins_head->formal_num.size(); ++i)
				{
					if (ins_head->formal_is_imm[i] == true)
						continue;
					if (ins_head->formal_num[i] == now_register->num)
						ins_head->formal_num[i] = new_register->num;
				}
			}
		}
	}
	Register_virtual* formal_arg = now_func->reg_head;
	while (formal_arg->next != NULL)
	{
		formal_arg = formal_arg->next;
		if (formal_arg->num == now_register->num)
			reg_copy(formal_arg, new_register);
	}
}

void get_copy_ins(instruction* now_ins, int num_reg_first, int num_reg_second)
{
	instruction* new_ins = new instruction;
	new_ins->num = now_ins->num;
	new_ins->op = ins_copy;
	new_ins->Rd = num_reg_first;
	new_ins->Rs1 = num_reg_second;
	new_ins->tRd = new_ins->tRs1 = i64;
	new_ins->next = now_ins->next;
	now_ins->next = new_ins;
}

/*将需要插入的copy指令插在对应指令之后*/
void add_copy_ins(Register_virtual* new_reg_first, Register_virtual* new_reg_second)
{
	vector<pair<int, int> >::iterator reg_it = new_reg_first->live_interval.begin();
	if (reg_it == new_reg_first->live_interval.end())
		return;
	int l1 = (*reg_it).first, r1 = (*reg_it).second;
	for (auto it : new_reg_second->live_interval)
	{
		int l2 = it.first, r2 = it.second;
		while (reg_it != new_reg_first->live_interval.end() && r1 < l2 - 1)
		{
			reg_it++;
			if (reg_it != new_reg_first->live_interval.end())
			{
				l1 = (*reg_it).first;
				r1 = (*reg_it).second;
			}
		}
		if (reg_it == new_reg_first->live_interval.end())
			return;
		if (r1 == l2 - 1)//reg2 = copy reg1
		{
			map_ins_copy[r1] = { new_reg_second->num,new_reg_first->num };
			get_copy_ins(map_instruction_position[r1], new_reg_second->num,
				new_reg_first->num);
		}
		if (l1 == r2 + 1)//reg1 = copy reg2
		{
			map_ins_copy[r2] = { new_reg_first->num,new_reg_second->num };
			get_copy_ins(map_instruction_position[r2], new_reg_first->num,
				new_reg_second->num);
		}
	}
}

/*尝试split*/
bool try_split(Register_virtual* now_reg, functions* now_func)
{
	if(now_reg->is_splited)
		return false;
	int op = (now_reg->type == float32) ? 1 : 0;
	Register_virtual* new_reg_first = NULL, * new_reg_second = NULL;
	type_spliting_weight mn_value = ULLONG_MAX;
	bool can_be_splited = false;
	for (auto it : physical_reg_order[op])
	{
		if (physical_reg_usable[op][it] == false)
			continue;
		Register_virtual* ret_reg_first = new Register_virtual, * ret_reg_second = new Register_virtual;
		/*划分出两个splited regs*/
		if (get_splited_reg(now_reg, reg[op][it].occupied, op * num_registers + it, now_func, ret_reg_first, ret_reg_second))
		{
			can_be_splited = true;
			type_spliting_weight value = min(ret_reg_first->spliting_weight, ret_reg_second->spliting_weight);
			if (value < mn_value || new_reg_first == NULL)
			{
				delete(new_reg_first); delete(new_reg_second);
				new_reg_first = ret_reg_first;
				new_reg_second = ret_reg_second;
				mn_value = value;
			}
		}
	}
	/*选择split方案*/
	/*继承原虚拟寄存器的类型*/
	if (new_reg_first == NULL)
		return false;
	/*加入寄存器列表*/
	add_new_register(new_reg_first, now_reg, now_func, 0);
	add_new_register(new_reg_second, now_reg, now_func, 1);
	change_instruction(now_reg, new_reg_first, now_func);
	change_instruction(now_reg, new_reg_second, now_func);
	add_copy_ins(new_reg_first, new_reg_second);
	return can_be_splited;
}

/*标记寄存器为spilled,为其分配一个空间*/
void spilled(Register_virtual* reg_head, functions* now_func)
{
	reg_head->is_spilled = true;
	/*新建一个变量*/
	variable_table* new_variable = new variable_table;
	new_variable->name = reg_head->name;
	new_variable->cnt = 1;
	new_variable->dim = 0;
	new_variable->num = ++total_global;
	new_variable->type = i64;
	new_variable->num_reg = reg_head->num;
	map_register_local[reg_head->num] = new_variable->num;
	map_variable_position[new_variable->num] = new_variable;
	now_func->local_tail->next = new_variable;
	now_func->local_tail = new_variable;
}

/*为用到的物理寄存器分配空间*/
void allocate_physical_reg(int physical_reg_num, functions* now_func)
{
	variable_table* new_variable = new variable_table;
	new_variable->name = "p" + std::to_string(physical_reg_num);
	new_variable->cnt = 1;
	new_variable->dim = 0;
	new_variable->num = ++total_global;
	new_variable->type = i64;
	new_variable->num_reg = 0;
	now_func->map_physical_register_local[physical_reg_num] = total_global;
	now_func->local_tail->next = new_variable;
	now_func->local_tail = new_variable;
}

void register_allocate(functions* now_func)
{
	allocate_clear();
	/*首先对每个寄存器计算spilting weight*/
	Register_virtual* reg_head = now_func->reg_head;
	while (reg_head->next != NULL)
	{
		reg_head = reg_head->next;
		reg_head->spliting_weight = calc_spliting_weight(reg_head, reg_head->num, now_func);
	}
	/*计算每个寄存器的优先级*/
	reg_head = now_func->reg_head;
	while (reg_head->next != NULL)
	{
		reg_head = reg_head->next;
		reg_head->prod = calc_prod(reg_head, now_func);
	}
	/*构建优先队列*/
	reg_head = now_func->reg_head;
	while (reg_head->next != NULL)//将寄存器插入优先队列
	{
		reg_head = reg_head->next;
		allocate_queue.push({ reg_head->prod,reg_head->num });
	}
	while (allocate_queue.size())
	{
		//if(map_global_register_position.count(allocate_queue.top().second)!=0)
		//	reg_head= map_global_register_position[allocate_queue.top().second];
		//else
		reg_head = map_local_register_position[allocate_queue.top().second];
		allocate_queue.pop();
		/*首次尝试分配*/
		if (try_assign(reg_head, now_func))
			continue;
		/*首次分配失败，拿出一个spliting weight更小的寄存器*/
		if (try_evction(reg_head, now_func))
			continue;
		/*若不是splited，则split，重新放回优先队列*/
		if (try_split(reg_head, now_func))
			continue;
		/*若splited，标记为spilled，不单独分配寄存器*/
		spilled(reg_head, now_func);
	}
	/*记录用到的物理寄存器和caller_saved寄存器*/
	reg_head = now_func->reg_head;
	while (reg_head->next != NULL)
	{
		reg_head = reg_head->next;
		if (reg_head->is_allocated == true)
		{
			int physical_reg_num = reg_head->reg_phisical;
			now_func->used_physical_reg[physical_reg_num] = true;
			if (physical_reg_saved[physical_reg_num / num_registers][physical_reg_num % num_registers] == caller_saved)
				now_func->caller_saved_reg[physical_reg_num] = true;
		}
	}

	check_register_allocate(now_func);

}

void get_callee_saved_reg()
{
	functions* now_func = func_head;
	while (now_func->next != NULL)
	{
		now_func = now_func->next;
		instruction* ins_head = now_func->ins_head;
		while (ins_head->next != NULL)
		{
			ins_head = ins_head->next;
			if (ins_head->op == ins_call)
			{
				int func_num = map_function[ins_head->name];
				functions* callee_func = map_function_position[func_num];
				/*get callee_saved regs*/
				for (int i = 0; i < (num_registers << 1); ++i)
				{
					if (now_func->used_physical_reg[i] == false)
						continue;
					if (physical_reg_saved[i / num_registers][i % num_registers] == callee_saved)
						callee_func->callee_saved_reg[i] = true;
				}
			}
		}
	}
}

void allocate_physical_reg(functions* now_func)
{
	/*分配callee_saved caller_saved的物理寄存器的栈空间*/
	for (int i = 0; i < (num_registers << 1); ++i)
	{
		if (now_func->callee_saved_reg[i] == true || now_func->caller_saved_reg[i] == true)
			allocate_physical_reg(i, now_func);
	}
}