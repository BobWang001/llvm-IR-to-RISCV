#include "riscv.h"

vector<basic_block*>bb_stack;//��������bb����ջ��,vectorģ��
set<int>bb_in_stack;//��¼��ջ�ڵ�bb
map<int, int>reg_definied;//��¼����Ĵ�������ֵ��λ��
map<int, int>reg_used;//��¼����Ĵ�����ʹ�õ�λ��
vector<int>reg_num;//��¼������ľֲ�����Ĵ���
extern set<int>ins_definied;//�ᶨ������Ĵ�����ָ��(��call)
extern set<int>ins_used;//��ʹ������Ĵ�����ָ��
vector<int>reg_in_loop;//��¼��ѭ���еļĴ���

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
	if (ins_used.find(ins_head->op) != ins_used.end())//Rs1����Ϊ�Ĵ���
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
	if (ins_head->op == ins_store)//storeָ���õ���Rd
	{
		reg_write(ins_head, ins_head->Rd, ins_head->num);
		if(ins_head->fimm==0)
			reg_write(ins_head, ins_head->Rs1, ins_head->num);
	}
	if (ins_head->op >= ins_add && ins_head->op <= ins_fcmp)//��Ҫ�ж�������
	{
		if (ins_head->fimm1 == 0)
			reg_write(ins_head, ins_head->Rs1, ins_head->num);
		if (ins_head->fimm2 == 0 && ins_head->op != ins_fneg)
			reg_write(ins_head, ins_head->Rs2, ins_head->num);
	}
	if (ins_head->op == ins_br && ins_head->branch_flag)//��Ҫ�ж�������ת
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
	if (ins_head->op == ins_call)//���������б�
	{
		int nw = 0;
		for (auto it : ins_head->formal_num)
		{
			if (ins_head->formal_is_imm[nw])
				continue;
			reg_write(ins_head, it, ins_head->num);
		}
	}
	if (ins_head->op == ins_ret && ins_head->type_ret != 2 && ins_head->fimm == false)//retָ����Ҫ�ж��Ƿ�Ϊvoid
		reg_write(ins_head, ins_head->Rs1, ins_head->num);
}

void get_used_in_loop(instruction* ins_head)
{
	if (ins_used.find(ins_head->op) != ins_used.end())//Rs1����Ϊ�Ĵ���
		reg_in_loop.push_back(ins_head->Rs1);
	if (ins_head->op == ins_store)//storeָ���õ���Rd
	{
		reg_in_loop.push_back(ins_head->Rd);
		if (ins_head->fimm == 0)
			reg_in_loop.push_back(ins_head->Rs1);
	}
	if (ins_head->op >= ins_add && ins_head->op <= ins_fcmp)//��Ҫ�ж�������
	{
		if (ins_head->fimm1 == 0)
			reg_in_loop.push_back(ins_head->Rs1);
		if (ins_head->fimm2 == 0 && ins_head->op != ins_fneg)
			reg_in_loop.push_back(ins_head->Rs2);
	}
	if (ins_head->op == ins_br && ins_head->branch_flag)//��Ҫ�ж�������ת
		reg_in_loop.push_back(ins_head->Rs1);
	if (ins_head->op == ins_call)//���������б�
	{
		int nw = 0;
		for (auto it : ins_head->formal_num)
		{
			if (ins_head->formal_is_imm[nw])
				continue;
			reg_in_loop.push_back(it);
		}
	}
	if (ins_head->op == ins_ret && ins_head->type_ret != 2 && ins_head->fimm == false)//retָ����Ҫ�ж��Ƿ�Ϊvoid
		reg_in_loop.push_back(ins_head->Rs1);
}

/*������Ĵ����ڲ����Ծ����*/
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
	if (now_bb->next == NULL)//��βbb
		return;
	if (bb_in_stack.find(now_bb->num) != bb_in_stack.end())//���ֻ�
	{
		bool find_loop = false;
		reg_in_loop.clear();
		reg_in_loop.shrink_to_fit();
		for (auto it : bb_stack)
		{
			if (it->num == now_bb->num)
				find_loop = true;
			if (find_loop)
			{
				it->is_loop = true;//���ѭ���ڵ�bb
				instruction* ins_head = it->ins_head;
				while (ins_head != NULL && ins_head->num <= it->r)
				{
					if (check_definied(ins_head))//�мĴ���������
						reg_in_loop.push_back(ins_head->Rd);
					get_used_in_loop(ins_head);//��ʹ�ù��ļĴ�����¼
					ins_head = ins_head->next;
				}
			}
		}
		find_loop = false;
		for (auto it : bb_stack)
		{
			if (it->num == now_bb->num)
				find_loop = true;
			if (find_loop)
			{
				for (auto it1 : reg_in_loop)
				{
					if (map_global_register_position.count(it1) != 0)
						add_live_interval(map_global_register_position[it1], it->l, it->r);
					else
						add_live_interval(now_func->map_local_register_position[it1], it->l, it->r);
				}
			}
		}
		return;
	}
	bb_stack.push_back(now_bb);
	bb_in_stack.insert(now_bb->num);
	instruction* ins_head = now_bb->ins_head;
	while (ins_head != NULL && ins_head->num <= now_bb->r)
	{
		if (check_definied(ins_head))//�мĴ���������
		{
			reg_definied[ins_head->Rd] = ins_head->num;//��¼�������λ��
			reg_num.push_back(ins_head->Rd);
		}
		get_used(ins_head);//��ʹ�ù��ļĴ�����¼
		ins_head = ins_head->next;
	}
	for (auto it : reg_used)
	{
		int l = reg_definied[it.first], r = it.second;
		if (map_global_register_position.count(it.first) != 0)
			add_live_interval(map_global_register_position[it.first], l, r);
		else
			add_live_interval(now_func->map_local_register_position[it.first], l, r);//���������Ĵ�����Ծ����
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
	sort(lst_live_interval.begin(), lst_live_interval.end(), cmp);//�Ƚ���������
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
			if (l > R + 1)//����������(�����󲢼�)
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

void check_live_interval(Register_virtual* now_reg)//������Ϣ
{
	printf("#reg_name=%s reg_number=%d live_intervals: ", now_reg->name.c_str(), now_reg->num);
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
	for (auto it : map_global)//�Ȳ���ȫ�ֱ�������ֵ��λ��
		reg_definied[it.second] = bb_head->ins_head->num;
	dfs(now_func, bb_head);
	for (auto it : reg_num)
	{
		Register_virtual* now_reg = now_func->map_local_register_position[it];
		resize_live_interval(now_reg);//����Ĵ����Ļ�Ծ����(���ϲ�)
		//check_live_interval(now_reg);
	}
	
	clear();
}