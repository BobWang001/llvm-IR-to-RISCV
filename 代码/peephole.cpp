#include "riscv.h"

extern set<int>asm_change_Rd;//会改变Rd的指令

void insert_ins(asm_instruction* asm_prev, asm_instruction* asm_next)
{
	asm_next->next = asm_prev->next;
	asm_prev->next->prev = asm_next;
	asm_next->prev = asm_prev;
	asm_prev->next = asm_next;
}

asm_instruction* remove_ins(asm_instruction* asm_ins)
{
	asm_instruction* pre_ins = asm_ins;
	asm_ins = asm_ins->next;
	pre_ins->prev->next = pre_ins->next;
	pre_ins->next->prev = pre_ins->prev;
	delete(pre_ins);
	return asm_ins;
}

bool change_Rd(asm_instruction* asm_ins, int Rd)
{
	if (asm_ins->op == ins_asm_label)
		return true;
	if (asm_change_Rd.find(asm_ins->op) != asm_change_Rd.end())
	{
		if (asm_ins->Rd == Rd)
			return true;
	}
	return false;
}

/*删除mv x,x*/
bool peephole_mv_x_x(functions* now_func)
{
	bool change = false, removed = false;
	asm_instruction* asm_head = now_func->asm_ins_head;
	while (asm_head->next != NULL)
	{
		if (!removed)
		asm_head = asm_head->next;
		removed = false;
		if (asm_head->op == ins_asm_mv)
		{
			if (asm_head->Rd == asm_head->Rs1)
			{
				asm_head = remove_ins(asm_head);
				change |= true;
				removed = true;
				if (asm_head == NULL)
					break;
			}
		}
	}
	return change;
}

/*删除无用的lw,ld,flw指令*/
bool peephole_ld(functions* now_func)
{
	bool change = false;
	asm_instruction* asm_head = now_func->asm_ins_head;
	while (asm_head->next != NULL)
	{
		asm_head = asm_head->next;
		if (asm_head->op == ins_asm_sw || asm_head->op == ins_asm_sd || asm_head->op == ins_asm_fsw)
		{
			int Rd_num = asm_head->Rd, Rs_num = asm_head->Rs1, imm = asm_head->imm;
			asm_instruction* asm_ins = asm_head;
			bool removed = false;
			while (asm_ins != NULL)
			{
				if (change_Rd(asm_ins, Rd_num))
					break;
				removed = false;
				if (asm_ins->op == ins_asm_lw || asm_ins->op == ins_asm_ld || asm_ins->op == ins_asm_flw)
				{
					if (asm_ins->Rs1 == Rs_num && asm_ins->imm == imm)
					{
						change |= true;
						asm_instruction* asm_new = new asm_instruction;
						asm_new->num = ++tot_asm_instructions;
						if (asm_ins->op != ins_asm_flw)
						{
							asm_new->op = ins_asm_mv;
							asm_new->Rd = asm_ins->Rd;
							asm_new->Rs1 = Rd_num;
						}
						else
						{
							asm_new->op = ins_asm_fadd_s;
							asm_new->Rd = asm_ins->Rd;
							asm_new->Rs1 = Rd_num;
							asm_new->Rs2 = zero;
						}
						insert_ins(asm_ins, asm_new);
						asm_ins = remove_ins(asm_ins);
						removed = true;
					}
				}
				if (!removed)
					asm_ins = asm_ins->next;
			}
		}
	}
	return change;
}

/*删除mv x,y后跟的mv y,x指令*/
bool peephole_mv(functions* now_func)
{
	bool change = false;
	asm_instruction* asm_head = now_func->asm_ins_head;
	while (asm_head->next != NULL)
	{
		asm_head = asm_head->next;
		if (asm_head->op == ins_asm_mv || (asm_head->op == ins_asm_fadd_s && asm_head->Rs2 == zero))
		{
			int Rd_num = asm_head->Rd, Rs_num = asm_head->Rs1;
			asm_instruction* asm_ins = asm_head;
			bool removed = false;
			while (asm_ins != NULL)
			{
				if (change_Rd(asm_ins, Rd_num))
					break;
				removed = false;
				if (asm_ins->op == ins_asm_mv || (asm_head->op == ins_asm_fadd_s && asm_head->Rs2 == zero))
				{
					if (asm_ins->Rd == Rs_num && asm_ins->Rs1 == Rd_num)
					{
						change |= true;
						asm_ins = remove_ins(asm_ins);
						removed = true;
					}
				}
				if (!removed)
					asm_ins = asm_ins->next;
			}
		}
	}
	return change;
}

/*修改无用的j指令*/
bool peephole_jump(functions* now_func)
{
	bool change = false, removed = false;
	asm_instruction* asm_head = now_func->asm_ins_head;
	while (asm_head->next != NULL)
	{
		if(!removed)
			asm_head = asm_head->next;
		removed = false;
		if (asm_head->op == ins_asm_j)
		{
			int label_num = asm_head->label;
			if (asm_head->next != NULL)
			{
				if (asm_head->next->op == ins_asm_label && asm_head->next->label == label_num)
				{
					change |= true;
					asm_head = remove_ins(asm_head);
					removed = true;
					continue;
				}
			}
			asm_instruction* asm_ins = now_func->asm_ins_head;
			while (asm_ins->next != NULL)
			{
				asm_ins = asm_ins->next;
				if (asm_ins->op == ins_asm_label && asm_ins->label == label_num)
				{
					asm_instruction* asm_next = asm_ins->next;
					if (asm_next != NULL)
					{
						if (asm_next->op == ins_asm_j)
						{
							change |= true;
							asm_head->label = asm_next->label;
						}
					}
					break;
				}
			}
		}
	}
	return change;
}

/*删除无用运算指令*/
bool peephole_operation(functions* now_func)
{
	bool change = false, removed = false;
	asm_instruction* asm_head = now_func->asm_ins_head;
	while (asm_head->next != NULL)
	{
		if (!removed)
			asm_head = asm_head->next;
		removed = false;
		if (asm_head->op == ins_asm_addi || asm_head->op == ins_asm_addiw ||
			asm_head->op == ins_asm_subi || asm_head->op == ins_asm_subiw)
		{
			if (asm_head->imm == 0)
			{
				asm_head = remove_ins(asm_head);
				change |= true;
				removed = true;
				if (asm_head == NULL)
					break;
			}
		}
	}
	return change;
}

void peephole(functions* now_func)
{
	bool change = false;
	do
	{
		change = false;
		change |= peephole_mv_x_x(now_func);
		change |= peephole_ld(now_func);
		change |= peephole_operation(now_func);
		change |= peephole_mv(now_func);
		change |= peephole_jump(now_func);
		change |= peephole_mv(now_func);
		change |= peephole_mv_x_x(now_func);
	} while (change);
}