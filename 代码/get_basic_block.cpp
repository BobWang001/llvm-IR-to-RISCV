#include "riscv.h"

extern vector<basic_block*>label_bb;//label编号对应的bb编号
extern map<int, int>label_ins;//label到指令的映射

void check_basic_block(functions* now_func)
{
	printf(";name=%s number_of_bbs=%d\n", now_func->name.c_str(), now_func->cnt_bb);
	basic_block* bb_head = now_func->bb_head;
	while (bb_head->next != NULL)
	{
		bb_head = bb_head->next;
		printf(";bb_number=%d l=%d r=%d edges:", bb_head->num, bb_head->l, bb_head->r);
		for (auto it : bb_head->edge)
		{
			printf("num=%d ", it->num);
		}
		printf("\n");
	}
	printf("\n");
}

void get_new_basic_block(int l, int r, functions* now_func)//起止指令
{
	basic_block* new_basic_block = new basic_block;
	new_basic_block->num = now_func->cnt_bb++;//记录编号
	new_basic_block->l = l;
	new_basic_block->r = r;
	now_func->bb_tail->next = new_basic_block;
	now_func->bb_tail = new_basic_block;
}

void add_edge(basic_block* x, basic_block* y)//在bb之间建边(x->y)
{
	x->edge.push_back(y);
}

void get_basic_block(functions* now_func)
{
	set<int>ins_flag;//对bb的终点指令打上标记
	ins_flag.clear();
	/*划分bb*/
	instruction* ins_head = now_func->ins_head;//遍历指令
	while (ins_head->next != NULL)
	{
		ins_head = ins_head->next;
		if (ins_head->op == ins_br)//分支指令
		{
			ins_flag.insert(ins_head->num);
			ins_flag.insert(label_ins[ins_head->L1]-1);
			if(ins_head->branch_flag)
				ins_flag.insert(label_ins[ins_head->L2] - 1);
		}
	}
	get_new_basic_block(0, 0, now_func);//起始块
	ins_head = now_func->ins_head;//遍历指令
	int ins_begin = 0;//记录bb起始指令编号
	while (ins_head->next != NULL)
	{
		ins_head = ins_head->next;
		if (!ins_begin)
			ins_begin = ins_head->num;
		if (ins_flag.find(ins_head->num)!=ins_flag.end())
		{
			get_new_basic_block(ins_begin, ins_head->num, now_func);//建立新的bb
			ins_begin = 0;
		}
		ins_head->num_bb = now_func->cnt_bb;//指令所在的bb编号
	}
	if (ins_begin)
		get_new_basic_block(ins_begin, ins_head->num, now_func);
	get_new_basic_block(0, 0, now_func);//终止块
	basic_block* bb_head = now_func->bb_head;
	ins_head = now_func->ins_head;
	ins_head = ins_head->next;//移到bb起始指令
	bb_head = bb_head->next;
	while (bb_head->next != now_func->bb_tail)
	{
		bb_head = bb_head->next;
		bb_head->ins_head = ins_head;
		while (ins_head != NULL && ins_head->num <= bb_head->r)
		{
			if (ins_head->op == ins_label)
				label_bb[ins_head->L1] = bb_head; //记录label所在的bb指针
			ins_head = ins_head->next;
		}
	}

	/*在bb之间连边*/
	bb_head = now_func->bb_head;
	ins_head = now_func->ins_head;
	ins_head = ins_head->next;//移到bb起始指令
	bb_head = bb_head->next;
	add_edge(bb_head, bb_head->next);
	while (bb_head->next != now_func->bb_tail)
	{
		bb_head = bb_head->next;
		while (ins_head != NULL && ins_head->num <= bb_head->r)
		{

			//printf("op=%d num=%d\n", ins_head->op, ins_head->num);

			if (ins_head->op == ins_br)
			{
				add_edge(bb_head, label_bb[ins_head->L1]);
				if (ins_head->branch_flag)//条件跳转
					add_edge(bb_head, label_bb[ins_head->L2]);
			}
			ins_head = ins_head->next;
		}
		if (bb_head->edge.size() == 0 || bb_head->next == now_func->bb_tail)
			add_edge(bb_head, bb_head->next);
	}
	ins_flag.clear();

	check_basic_block(now_func);

}