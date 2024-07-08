#include "riscv.h"
using namespace std;
/*
������ƹ��ɣ�
ȫ�ֱ�����G+���
������F+���
��ת��ţ�L+���

�Ĵ�������߼���
��Rd������̶�ѡ��t0/ft0��Rs1������̶�ѡ��t1/ft1��Rs2������̶�ѡ��t2/ft2

������������12λ��ѡ��ra�洢����������>=2�����������Ra�⣬����Ҫ��Rs1�Ĵ���
��������Ϊ���������ȴ���ra����ת�浽����Ĵ���(Rs1��Rs2)
*/
extern variable_table* global, * global_tail;
extern int total_global;//�洢����������
extern map<type_variables, int>map_global;//ȫ�ֱ���������ŵ�ӳ��
extern map<int, int>map_register_local;//�Ĵ�����ŵ�������ŵ�ӳ��
extern map<int, int>map_register_local_alloca;//alloca����ڼĴ�����ŵ�������ŵ�ӳ��

extern instruction* start;//����ȫ�ֵ�ָ��
extern map<std::string, int>ins_num, cond_num;
extern map<int, instruction*>map_instruction_position;//ָ���ŵ�ָ���ӳ��
extern map<int, pair<int, int> >map_ins_copy;//��¼copyָ���λ��

extern map<type_label, int>label_num;//label������ŵ�ӳ��
extern map<int, int>label_ins;//label��ָ���ӳ��
extern vector<basic_block*>label_bb;//label��Ŷ�Ӧ��bb���
extern int tot_label;//label����

extern functions* func_head, * func_tail;
extern map<type_label, int>map_function;
extern int tot_functions;//�ܵĺ�����
int tot_asm_instructions;//�ܵĻ��ָ����

bool occupied_reg[2][num_registers];

void clear_occupied_reg()
{
	for (int i = 0; i < num_registers; ++i)
		occupied_reg[0][i] = occupied_reg[1][i] = false;
}

float BinaryToFloat(unsigned int num)//��������ת��Ϊ��Ӧ�ĸ�����
{
	// ʹ��union������float��λ��ʾ
	union
	{
		float f;
		unsigned int i;
	}b;
	b.i = num;
	std::bitset<32> bs(b.i); // ����һ��32λ��bitset
	float ret = 1.0;
	int fl = bs[31];//����λ
	int imm = 0;
	for (int i = 29; i >= 23; --i)//����
		imm += bs[i] * (1 << (i - 23));
	if (bs[30] == 0)
		imm -= (1 << 7);
	for (int i = 22; i >= 0; --i)//β��
		ret += bs[i] * powf(2, i - 23);
	if (fl)
		ret *= -1;
	ret = ret * powf(2, imm);
	return ret;
}

/*����ָ��*/
void get_new_asm_instruction(int op, int Rd, int Rs1, int Rs2, int imm, int label, functions* now_func)
{
	asm_instruction* new_ins = new asm_instruction;
	new_ins->num = ++tot_asm_instructions;
	new_ins->op = op;
	new_ins->Rd = Rd;
	new_ins->Rs1 = Rs1;
	new_ins->Rs2 = Rs2;
	new_ins->imm = imm;
	new_ins->label = label;
	now_func->asm_ins_tail->next = new_ins;
	new_ins->prev = now_func->asm_ins_tail;
	now_func->asm_ins_tail = new_ins;
}

/*�ݴ�Ĵ����ڵ�����*/
void temp_store(int op, int reg_num, int imm, functions* now_func)
{
	if(op)
		get_new_asm_instruction(ins_asm_fsd, reg_num, sp, -1, imm, -1, now_func);
	else
		get_new_asm_instruction(ins_asm_sd, reg_num, sp, -1, imm, -1, now_func);
}

/*��ȡ����Ĵ�����Ӧ��ַ������*/
void temp_load(int op, int reg_num, int imm, functions* now_func)
{
	if (op)
		get_new_asm_instruction(ins_asm_fld, reg_num, sp, -1, imm, -1, now_func);
	else
		get_new_asm_instruction(ins_asm_ld, reg_num, sp, -1, imm, -1, now_func);
}

/*��ȡȫ�ּĴ�����Ӧ��ַ������*/
void temp_global(int reg_num, int imm, functions* now_func)
{
	get_new_asm_instruction(ins_asm_la, reg_num, -1, -1, -1, imm, now_func);
}

/*�ж��������Ƿ���12λ����*/
bool is_in_range(unsigned int imm)
{
	if (imm >= (1 << 12))
		return false;
	else return true;
}

bool is_free(int num, int op, int reg_num)
{
	for (auto it : reg[op][reg_num].occupied)
	{
		int l = it.first, r = it.second;
		if (num >= l && num <= r)
			return true;//�ѱ�ռ��
	}
	return false;//δ��ռ��
}

/*callָ��ǰ����*/
void save_caller_saved_regs(functions* now_func)
{
	for (int i = 0; i < (num_registers << 1); ++i)
	{
		if (now_func->caller_saved_reg[i] == true)
		{
			int imm = now_func->imm_local[now_func->map_physical_register_local[i]];
			if (i >= num_registers)
				get_new_asm_instruction(ins_asm_fsd, i, sp, -1, imm, -1, now_func);
			else
				get_new_asm_instruction(ins_asm_sd, i, sp, -1, imm, -1, now_func);
		}
	}
}

/*callָ���ȡ��*/
void load_caller_saved_regs(functions* now_func, int R1 = -1)
{
	for (int i = 0; i < (num_registers << 1); ++i)
	{
		if (i == R1)
			continue;
		if (now_func->caller_saved_reg[i] == true)
		{
			int imm = now_func->imm_local[now_func->map_physical_register_local[i]];
			if (i >= num_registers)
				get_new_asm_instruction(ins_asm_fld, i, sp, -1, imm, -1, now_func);
			else
				get_new_asm_instruction(ins_asm_ld, i, sp, -1, imm, -1, now_func);
		}
	}
}

/*������ʼλ�ñ���*/
void save_callee_saved_regs(functions* now_func)
{
	for (int i = 0; i < (num_registers << 1); ++i)
	{
		if (now_func->callee_saved_reg[i] == true)
		{
			int imm = now_func->imm_local[now_func->map_physical_register_local[i]];
			if (i >= num_registers)
				get_new_asm_instruction(ins_asm_fsd, i, sp, -1, imm, -1, now_func);
			else
				get_new_asm_instruction(ins_asm_sd, i, sp, -1, imm, -1, now_func);
		}
	}
}

/*��������ǰȡ��*/
void load_callee_saved_regs(functions* now_func)
{
	for (int i = 0; i < (num_registers << 1); ++i)
	{
		if (now_func->callee_saved_reg[i] == true)
		{
			int imm = now_func->imm_local[now_func->map_physical_register_local[i]];
			if (i >= num_registers)
				get_new_asm_instruction(ins_asm_fld, i, sp, -1, imm, -1, now_func);
			else
				get_new_asm_instruction(ins_asm_ld, i, sp, -1, imm, -1, now_func);
		}
	}
}

/*�����ҵ���������Ĵ���*/
bool find_free_reg(int& now_reg, int num, int op, int R1 = -1, int R2 = -1)
{
	for (int i = 0; i < num_registers; ++i)
	{
		if (op * num_registers + i == R1)
			continue;
		if (op * num_registers + i == R2)
			continue;
		if (!physical_reg_usable[op][i])
			continue;
		bool fl = true;
		for (auto it : reg[op][i].occupied)
		{
			int l = it.first, r = it.second;
			if (num >= l && num <= r)
			{
				fl = false;
				break;
			}
		}
		if (fl == true)
		{
			now_reg = op * num_registers + i;
			return true;
		}
	}
	return false;
}

void find_physical_reg(int op, int ins_num, int virtual_reg, int num_not_find, int& physical_reg, bool& fReg,
	int imm = 0, int R1 = -1, int R2 = -1)
{
	if (map_global_register_position.count(virtual_reg) != 0)//ȫ�ּĴ���
	{
		if (op)
		{
			fReg = find_free_reg(physical_reg, ins_num, op);
			if (!fReg)
				physical_reg = num_not_find;
		}
		else
		{
			if (is_in_range(imm) && R1 != ra && R2 != ra)
			{
				physical_reg = ra;
				fReg = true;
			}
			else
			{
				fReg = find_free_reg(physical_reg, ins_num, op, R1, R2);
				if (!fReg)
					physical_reg = num_not_find;
			}
		}
	}
	else if (map_local_register_position[virtual_reg]->is_spilled)
	{
		if (op)
		{
			fReg = find_free_reg(physical_reg, ins_num, op);
			if (!fReg)
				physical_reg = num_not_find;
		}
		else
		{
			if (is_in_range(imm) && R1 != ra && R2 != ra)
			{
				physical_reg = ra;
				fReg = true;
			}
			else
			{
				fReg = find_free_reg(physical_reg, ins_num, op, R1, R2);
				if (!fReg)
					physical_reg = num_not_find;
			}
		}
	}
	else
	{
		physical_reg = map_local_register_position[virtual_reg]->reg_phisical;
		fReg = true;
	}
}

void pre_load_store(functions* now_func, int vRd = -1, int fRd = -1, int Rd = -1,
	int vRs1 = -1, int fRs1 = -1, int Rs1 = -1, int vRs2 = -1, int fRs2 = -1, int Rs2 = -1)
{
	if (Rd != -1)
	{
		int op = (Rd >= num_registers) ? 1 : 0;
		if (!fRd)
			temp_store(op, Rd, delta_Rd, now_func);
		if (vRd != -1)
		{
			if (map_global_register_position.count(vRd) != 0)//ȫ�ּĴ���
				temp_global(Rd, vRd, now_func);
			if (map_local_register_position[vRd]->is_spilled)
				temp_load(op, Rd, now_func->imm_local[map_register_local[vRd]], now_func);
		}
	}
	if (Rs1 != -1)
	{
		int op = (Rs1 >= num_registers) ? 1 : 0;
		if (!fRs1)
			temp_store(op, Rs1, delta_Rs1, now_func);
		if (vRs1 != -1)
		{
			if (map_global_register_position.count(vRs1) != 0)//ȫ�ּĴ���
				temp_global(Rs1, vRs1, now_func);
			else if (map_local_register_position[vRs1]->is_spilled)
				temp_load(op, Rs1, now_func->imm_local[map_register_local[vRs1]], now_func);
		}
	}
	if (Rs2 != -1)
	{
		int op = (Rs2 >= num_registers) ? 1 : 0;
		if (!fRs2)
			temp_store(op, Rs2, delta_Rs2, now_func);
		if (vRs2 != -1)
		{
			if (map_global_register_position.count(vRs2) != 0)//ȫ�ּĴ���
				temp_global(Rs2, vRs2, now_func);
			else if (map_local_register_position[vRs2]->is_spilled)
				temp_load(op, Rs2, now_func->imm_local[map_register_local[vRs2]], now_func);
		}
	}
}

void suf_load_store(functions* now_func, int vRd = -1, int fRd = -1, int Rd = -1,
	int vRs1 = -1, int fRs1 = -1, int Rs1 = -1, int vRs2 = -1, int fRs2 = -1, int Rs2 = -1)
{
	if (Rd != -1)
	{
		int op = (Rd >= num_registers) ? 1 : 0;
		if (map_global_register_position.count(vRd) == 0 && vRd != -1 &&
			map_local_register_position[vRd]->is_spilled)
			temp_store(op, Rd, now_func->imm_local[map_register_local[vRd]], now_func);
		if (!fRd)
			temp_load(op, Rd, delta_Rd, now_func);
	}
	if (Rs1 != -1)
	{
		int op = (Rs1 >= num_registers) ? 1 : 0;
		if (map_global_register_position.count(vRs1) == 0 && vRs1 != -1 &&
			map_local_register_position[vRs1]->is_spilled)
			temp_store(op, Rs1, now_func->imm_local[map_register_local[vRs1]], now_func);
		if (!fRs1)
			temp_load(op, Rs1, delta_Rs1, now_func);
	}
	if (Rs2 != -1)
	{
		int op = (Rs2 >= num_registers) ? 1 : 0;
		if (map_global_register_position.count(vRs2) == 0 && vRs2 != -1 &&
			map_local_register_position[vRs2]->is_spilled)
			temp_store(op, Rs2, now_func->imm_local[map_register_local[vRs2]], now_func);
		if (!fRs2)
			temp_load(op, Rs2, delta_Rs2, now_func);
	}
}

void gen_load(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs = now_ins->Rs1, imm = (int)now_ins->imm;
	int Rd, Rs;
	int op = (now_ins->tRd == float32) ? 1 : 0;
	bool fRd = false, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	/*ȷ������Ĵ���*/
	find_physical_reg(op, now_ins->num, vRd, (op) ? ft0 : t0, Rd, fRd, imm);
	find_physical_reg(0, now_ins->num, vRs, t1, Rs, fRs, imm, Rd);
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
	/*�������������ָ��*/
	if (!is_in_range(imm))
	{
		/*��ȡ������*/
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
		/*��ȡ�ڴ��ַ*/
		get_new_asm_instruction(ins_add, Rs, ra, Rs, -1, -1, now_func);
		imm = 0;
	}
	/*���ָ��*/
	switch (now_ins->tRd)
	{
		case i32:
		{
			get_new_asm_instruction(ins_asm_lw, Rd, Rs, -1, imm, -1, now_func);
			break;
		}
		case i64:
		{
			get_new_asm_instruction(ins_asm_ld, Rd, Rs, -1, imm, -1, now_func);
			break;
		}
		case float32:
		{
			get_new_asm_instruction(ins_asm_flw, Rd, Rs, -1, imm, -1, now_func);
			break;
		}		
	}
	/*�ָ��Ĵ���ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
}

void gen_store(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs = now_ins->Rs1, imm = (int)now_ins->imm;
	int Rd, Rs;
	int op = (now_ins->tRs1 == float32) ? 1 : 0;
	bool fRd = false, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	/*ȷ������Ĵ���*/
	if (now_ins->fimm == false)
		find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, imm);
	else
		find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, imm, ra);
	if (now_ins->fimm == false)
		find_physical_reg(op, now_ins->num, vRs, (op) ? ft1 : t1, Rs, fRs, imm, Rd);
	else
		vRs = Rs = -1;
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
	if (now_ins->fimm == false)
	{
		/*�������������ָ��*/
		if (!is_in_range(imm))
		{
			/*��ȡ������*/
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
			/*��ȡ�ڴ��ַ*/
			get_new_asm_instruction(ins_add, Rd, ra, Rd, -1, -1, now_func);
			imm = 0;
		}
		/*���ָ��*/
		switch (now_ins->tRd)
		{
			case i32:
			{
				get_new_asm_instruction(ins_asm_sw, Rs, Rd, -1, imm, -1, now_func);
				break;
			}
			case i64:
			{
				get_new_asm_instruction(ins_asm_sd, Rs, Rd, -1, imm, -1, now_func);
				break;
			}
			case float32:
			{
				get_new_asm_instruction(ins_asm_fsw, Rs, Rd, -1, imm, -1, now_func);
				break;
			}
		}
	}
	else
	{
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
		if (now_ins->tRs1 == float32)
			get_new_asm_instruction(ins_asm_fsw, ra, Rd, -1, 0, -1, now_func);
		else if (now_ins->tRs1 == i32)
			get_new_asm_instruction(ins_asm_sw, ra, Rd, -1, 0, -1, now_func);
		if (now_ins->tRs1 == i64)
			get_new_asm_instruction(ins_asm_sd, ra, Rd, -1, 0, -1, now_func);
	}
	/*�ָ��Ĵ���ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
}

void gen_alloca(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd;
	int Rd, Rs = -1;
	variable_table* now_variable = map_variable_position[map_register_local_alloca[vRd]];
	int op = (now_variable->type == float32) ? 1 : 0;
	bool fRd = false, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	if (now_variable->val.size() != 0)//��Ҫһ����ʱ�Ĵ���������
	{
		if (op)
		{
			fRs = find_free_reg(Rs, now_ins->num, 1, -1, -1);
			if (!fRs)
				Rs = ft1;
		}
		else
		{
			Rs = ra;
			fRs = true;
		}
	}
	/*ȷ������Ĵ���*/
	find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, 0, Rs, ra);
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, -1, fRs, Rs);
	/*װ������׵�ַ*/
	int imm = now_func->imm_local[map_register_local_alloca[vRd]];
	if (is_in_range(imm))
	{
		get_new_asm_instruction(ins_asm_addi, Rd, sp, -1, imm, -1, now_func);
	}
	else
	{
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
		get_new_asm_instruction(ins_asm_add, Rd, ra, sp, -1, -1, now_func);
	}
	/*�������ֵ*/
	for (auto it : now_variable->val)
	{
		int imm_i32 = it;
		unsigned int imm_float = it;
		switch (op)
		{
			case i32:
			{
				get_new_asm_instruction(ins_asm_la, Rs, -1, -1, imm_i32, -1, now_func);
				get_new_asm_instruction(ins_asm_sw, Rs, sp, -1, imm, -1, now_func);
				imm += 4;
				break;
			}
			case float32:
			{
				get_new_asm_instruction(ins_asm_la, ra, -1, -1, imm_i32, -1, now_func);
				get_new_asm_instruction(ins_asm_fmv_w_x, Rs, ra, -1, -1, -1, now_func);
				get_new_asm_instruction(ins_asm_fsw, Rs, sp, -1, imm, -1, now_func);
				imm += 4;
				break;
			}
		}
	}
	/*�ָ��Ĵ���ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, -1, fRs, Rs);
}

void gen_GEP(instruction* now_ins, functions* now_func)
{
	if (now_ins->all_imm)
	{
		int vRd = now_ins->Rd, vRs = now_ins->Rs1;
		int Rd, Rs;
		int imm = now_ins->imm;
		bool fRd = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
		/*ȷ������Ĵ���*/
		find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd);
		/*�ݴ��Լ�װ��*/
		pre_load_store(now_func, vRd, fRd, Rd);
		if (map_global_register_position.count(vRs) != 0)//ȫ�ֱ�����ַ
		{
			temp_global(Rd, map_global_register_position[vRs]->num, now_func);
			if (is_in_range(imm))
				get_new_asm_instruction(ins_asm_addi, Rd, Rd, -1, imm, -1, now_func);
			else
			{
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
				get_new_asm_instruction(ins_asm_add, Rd, Rd, ra, -1, -1, now_func);
			}
		}
		else//�ֲ�������ַ
		{
			int address = now_func->imm_local[map_register_local_alloca[vRs]];//��ȡ�����׵�ַ
			imm += address;
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
			get_new_asm_instruction(ins_asm_add, Rd, ra, sp, -1, -1, now_func);
		}
		/*�ָ��Ĵ���ԭ��ֵ*/
		suf_load_store(now_func, vRd, fRd, Rd);
	}
	else
	{
		int vRd = now_ins->Rd, vRs1 = now_ins->Rs1;
		int Rd, Rs1;
		bool fRd = false, fRs1 = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
		/*ȷ������Ĵ���*/
		find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, 0, ra);
		/*�ݴ��Լ�װ��*/
		pre_load_store(now_func, vRd, fRd, Rd);
		if (map_global_register_position.count(vRs1) != 0)//ȫ�ֱ�����ַ
			temp_global(Rd, map_global_register_position[vRs1]->num, now_func);
		else//�ֲ�������ַ
		{
			int address = now_func->imm_local[map_register_local_alloca[vRs1]];//��ȡ�����׵�ַ
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, address, -1, now_func);
			get_new_asm_instruction(ins_asm_add, Rd, ra, sp, -1, -1, now_func);
		}
		int nw = 0, res = now_ins->size;
		for (auto it : now_ins->formal_num)
		{
			int size = now_ins->gep_size[nw];
			if (now_ins->formal_is_imm[nw])//��ά��Ϊ������
			{
				int imm = res * it;
				imm <<= 2;
				if (imm != 0)
				{
					if (is_in_range(imm))
						get_new_asm_instruction(ins_asm_addi, Rd, Rd, -1, imm, -1, now_func);
					else
					{
						get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
						get_new_asm_instruction(ins_asm_add, Rd, Rd, ra, -1, -1, now_func);
					}
				}
			}
			else//��ά��Ϊ�Ĵ�����ֵ
			{
				vRs1 = it;
				fRs1 = false;
				/*ȷ������Ĵ���*/
				find_physical_reg(0, now_ins->num, vRs1, t1, Rs1, fRs1, 0, ra, Rd);
				/*�ݴ��Լ�װ��*/
				pre_load_store(now_func, vRs1, fRs1, Rs1);
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, res, -1, now_func);
				get_new_asm_instruction(ins_asm_mul, ra, ra, Rs1, -1, -1, now_func);
				get_new_asm_instruction(ins_asm_slli, ra, ra, -1, 2, -1, now_func);
				get_new_asm_instruction(ins_asm_add, Rd, Rd, ra, -1, -1, now_func);
				/*�ָ��Ĵ���ԭ��ֵ*/
				suf_load_store(now_func, vRs1, fRs1, Rs1);
			}
			nw++;
			res /= size;
		}
		/*�ָ��Ĵ���ԭ��ֵ*/
		suf_load_store(now_func, vRd, fRd, Rd);
	}
}

void gen_operation(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs1 = now_ins->Rs1, vRs2 = now_ins->Rs2;
	int Rd, Rs1, Rs2;
	int imm1 = now_ins->imm1, imm2 = now_ins->imm2, imm = 0;
	float imm1_f = 0, imm2_f = 0, imm_f = 0;
	int op = now_ins->tRd;
	bool fRd = false, fRs1 = false, fRs2 = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	bool need_3_regs = false;//�Ƿ��õ��������Ĵ���
	int num_imm = 0;
	if (now_ins->fimm1)
		num_imm++;
	if (now_ins->fimm2)
		num_imm++;
	if (now_ins->op == ins_fneg)//��������fnegָ��
	{
		if (now_ins->fimm1)//������
		{
			imm_f = BinaryToFloat(imm);
			imm_f *= -1;
			imm = floatToBinary(imm_f);
			/*ȷ������Ĵ���*/
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRd, (op == float32) ? ft0 : t0, Rd, fRd, 0, ra);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd);
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
			get_new_asm_instruction(ins_asm_fmv_w_x, Rd, ra, -1, -1, -1, now_func);
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd);
		}
		else
		{
			/*ȷ������Ĵ���*/
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRd, (op == float32) ? ft0 : t0, Rd, fRd);
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs1, (op == float32) ? ft1 : t1, Rs1, fRs1);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1);
			get_new_asm_instruction(ins_asm_fneg_s, Rd, Rs1, -1, -1, -1, now_func);
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1);
		}
		return;
	}
	if (num_imm == 1)
	{
		imm = imm1 + imm2;
		if (!is_in_range(imm) || op == float32 || now_ins->op == ins_mul || now_ins->op == ins_sdiv
			|| now_ins->op == ins_srem)//��Ҫ��ʱ�Ĵ�����������
		{
			need_3_regs = true;
			if (op == float32)
			{
				fRs2 = find_free_reg(Rs2, now_ins->num, 1, Rd, Rs1);
				if (!fRs2)
					Rs2 = ft2;
			}
			else
			{
				Rs2 = ra;
				fRs2 = true;
			}
			/*������������Rs2*/
			if (op == float32)
			{
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
				get_new_asm_instruction(ins_asm_fmv_w_x, Rs2, ra, -1, -1, -1, now_func);
			}
			else
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);

		}
		else
			vRs2 = Rs2 = -1;
		if (!need_3_regs)
		{
			/*ȷ������Ĵ���*/
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRd, (op == float32) ? ft0 : t0, Rd, fRd, 0, Rs2);
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs1, (op == float32) ? ft1 : t1, Rs1, fRs1, 0, Rd, Rs2);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
			if (op == i64)
			{
				switch (now_ins->op)
				{
				case ins_add:
				{
					get_new_asm_instruction(ins_asm_addi, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_sub:
				{
					get_new_asm_instruction(ins_asm_subi, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_and:
				{
					get_new_asm_instruction(ins_asm_andi, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_or:
				{
					get_new_asm_instruction(ins_asm_ori, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_xor:
				{
					get_new_asm_instruction(ins_asm_xori, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				}
			}
			else if (op == i32)
			{
				switch (now_ins->op)
				{
				case ins_add:
				{
					get_new_asm_instruction(ins_asm_addiw, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_sub:
				{
					get_new_asm_instruction(ins_asm_subiw, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_and:
				{
					get_new_asm_instruction(ins_asm_andi, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_or:
				{
					get_new_asm_instruction(ins_asm_oriw, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				case ins_xor:
				{
					get_new_asm_instruction(ins_asm_xoriw, Rd, Rs1, -1, imm, -1, now_func);
					break;
				}
				}
			}
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
		}
	}
	if (num_imm == 0 || need_3_regs == true)//û��������
	{
		/*ȷ������Ĵ���*/
		find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRd, (op == float32) ? ft0 : t0, Rd, fRd);
		find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs1, (op == float32) ? ft1 : t1, Rs1, fRs1, 0, Rd);
		if (num_imm == 0)
		find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs2, (op == float32) ? ft2 : t2, Rs2, fRs2, 0, Rd, Rs1);
		/*�ݴ��Լ�װ��*/
		pre_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
		if (op == i64)
		{
			switch (now_ins->op)
			{
				case ins_add:
				{
					get_new_asm_instruction(ins_asm_add, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_sub:
				{
					get_new_asm_instruction(ins_asm_sub, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_mul:
				{
					get_new_asm_instruction(ins_asm_mul, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_sdiv:
				{
					get_new_asm_instruction(ins_asm_div, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_srem:
				{
					get_new_asm_instruction(ins_asm_rem, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_and:
				{
					get_new_asm_instruction(ins_asm_and, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_or:
				{
					get_new_asm_instruction(ins_asm_or, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_xor:
				{
					get_new_asm_instruction(ins_asm_xor, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
			}
		}
		else if (op == i32)
		{
			switch (now_ins->op)
			{
				case ins_add:
				{
					get_new_asm_instruction(ins_asm_addw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_sub:
				{
					get_new_asm_instruction(ins_asm_subw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_mul:
				{
					get_new_asm_instruction(ins_asm_mulw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_sdiv:
				{
					get_new_asm_instruction(ins_asm_divw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_srem:
				{
					get_new_asm_instruction(ins_asm_remw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_and:
				{
					get_new_asm_instruction(ins_asm_andw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_or:
				{
					get_new_asm_instruction(ins_asm_orw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_xor:
				{
					get_new_asm_instruction(ins_asm_xorw, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
			}
		}
		else if (op == float32)
		{
			switch (now_ins->op)
			{
				case ins_fadd:
				{
					get_new_asm_instruction(ins_asm_fadd_s, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_fsub:
				{
					get_new_asm_instruction(ins_asm_fsub_s, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_fmul:
				{
					get_new_asm_instruction(ins_asm_fmul_s, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_fdiv:
				{
					get_new_asm_instruction(ins_asm_fdiv_s, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
				case ins_frem:
				{
					get_new_asm_instruction(ins_asm_frem_s, Rd, Rs1, Rs2, -1, -1, now_func);
					break;
				}
			}
		}
		/*�ָ�ԭ��ֵ*/
		suf_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
	}
	else if (num_imm == 2)//ȫΪ������,ֱ����liָ��
	{
		if (op != float32)
		{
			switch (now_ins->op)
			{
			case ins_add:
			{
				imm = imm1 + imm2;
				break;
			}
			case ins_sub:
			{
				imm = imm1 - imm2;
				break;
			}
			case ins_mul:
			{
				imm = imm1 * imm2;
				break;
			}
			case ins_sdiv:
			{
				imm = imm1 / imm2;
				break;
			}
			case ins_srem:
			{
				imm = imm1 % imm2;
				break;
			}
			case ins_and:
			{
				imm = imm1 & imm2;
				break;
			}
			case ins_or:
			{
				imm = imm1 | imm2;
				break;
			}
			case ins_xor:
			{
				imm = imm1 ^ imm2;
				break;
			}
			}
			/*ȷ������Ĵ���*/
			find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, 0, ra);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd);
			/*���ɻ�����*/
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
			get_new_asm_instruction(ins_asm_mv, Rd, ra, -1, -1, -1, now_func);
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd);
		}
		else
		{
			imm1_f = BinaryToFloat(imm1);
			imm2_f = BinaryToFloat(imm2);
			switch (now_ins->op)
			{
			case ins_fadd:
			{
				imm_f = imm1_f + imm2_f;
				break;
			}
			case ins_fsub:
			{
				imm_f = imm1_f - imm2_f;
				break;
			}
			case ins_fmul:
			{
				imm_f = imm1_f * imm2_f;
				break;
			}
			case ins_fdiv:
			{
				imm_f = imm1_f / imm2_f;
				break;
			}
			case ins_frem:
			{
				imm_f = fmodf(imm1_f, imm2_f);
				break;
			}
			}
			imm = floatToBinary(imm_f);
			/*ȷ������Ĵ���*/
			find_physical_reg(1, now_ins->num, vRd, ft0, Rd, fRd, 0, ra);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd);
			/*���ɻ�����*/
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
			get_new_asm_instruction(ins_asm_fmv_w_x, Rd, ra, -1, -1, -1, now_func);
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd);
		}
	}
}

void gen_xcmp(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs1 = now_ins->Rs1, vRs2 = now_ins->Rs2;
	int Rd, Rs1, Rs2;
	int imm1 = now_ins->imm1, imm2 = now_ins->imm2;
	int op = now_ins->tRd;
	bool fRd = false, fRs1 = false, fRs2 = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	/*ȷ������Ĵ���*/
	if (op == float32)
	{
		if (now_ins->fimm1 || now_ins->fimm2)
			find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd, 0, ra);
		else
			find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd);
	}
	else
		Rd = -1;
	if (now_ins->fimm1 == true)//Rs1��Ҫ��������
	{
		fRs1 = find_free_reg(Rs1, now_ins->num, (op == float32) ? 1 : 0, Rd);
	}
	else
		find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs1, (op == float32) ? ft1 : t1, Rs1, fRs1, 0, Rd);
	if (now_ins->fimm2 == true)//Rs2��Ҫ��������
	{
		fRs2 = find_free_reg(Rs2, now_ins->num, (op == float32) ? 1 : 0, Rd, Rs1);
		if (!fRs2)
			Rs2 = (op == float32) ? ft2 : t2;
	}
	else
		find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs2, (op == float32) ? ft2 : t2, Rs2, fRs2, 0, Rd, Rs1);
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
	if (now_ins->fimm1 == true)//Rs1��Ҫ��������
	{
		if (op == float32)
		{
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm1, -1, now_func);
			get_new_asm_instruction(ins_asm_fmv_w_x, Rs1, ra, -1, -1, -1, now_func);
		}
		else
			get_new_asm_instruction(ins_asm_li, Rs1, -1, -1, imm1, -1, now_func);
	}
	if (now_ins->fimm2 == true)//Rs2��Ҫ��������
	{
		if (op == float32)
		{
			get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm2, -1, now_func);
			get_new_asm_instruction(ins_asm_fmv_w_x, Rs2, ra, -1, -1, -1, now_func);
		}
		else
			get_new_asm_instruction(ins_asm_li, Rs2, -1, -1, imm2, -1, now_func);
	}
	if (op != float32)//icmp,ͨ����һ��brָ���ȡ��ת��ַ
	{
		int cond = now_ins->cond;
		now_ins = now_ins->next;
		int L1 = now_ins->L1, L2 = now_ins->L2;
		switch (cond)
		{
			case cond_eq:
			{
				get_new_asm_instruction(ins_asm_beq, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_ne:
			{
				get_new_asm_instruction(ins_asm_bne, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_ugt:
			{
				get_new_asm_instruction(ins_asm_bgtu, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_sgt:
			{
				get_new_asm_instruction(ins_asm_bgt, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_ule:
			{
				get_new_asm_instruction(ins_asm_bleu, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_sle:
			{
				get_new_asm_instruction(ins_asm_ble, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_uge:
			{
				get_new_asm_instruction(ins_asm_bgeu, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_sge:
			{
				get_new_asm_instruction(ins_asm_bge, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_ult:
			{
				get_new_asm_instruction(ins_asm_bltu, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
			case cond_slt:
			{
				get_new_asm_instruction(ins_asm_blt, -1, Rs1, Rs2, -1, L1, now_func);
				break;
			}
		}
		get_new_asm_instruction(ins_asm_j, -1, -1, -1, -1, L2, now_func);
	}
	else
	{
		int cond = now_ins->cond;
		switch (cond)
		{
			case cond_oeq:
			{
				get_new_asm_instruction(ins_asm_feq_s, Rd, Rs1, Rs2, -1, -1, now_func);
				break;
			}
			case cond_ogt:
			{
				get_new_asm_instruction(ins_asm_flt_s, Rd, Rs2, Rs1, -1, -1, now_func);
				break;
			}
			case cond_oge:
			{
				get_new_asm_instruction(ins_asm_fle_s, Rd, Rs2, Rs1, -1, -1, now_func);
				break;
			}
			case cond_olt:
			{
				get_new_asm_instruction(ins_asm_flt_s, Rd, Rs1, Rs2, -1, -1, now_func);
				break;
			}
			case cond_ole:
			{
				get_new_asm_instruction(ins_asm_fle_s, Rd, Rs1, Rs2, -1, -1, now_func);
				break;
			}
			case cond_une://����nan�����
			{
				get_new_asm_instruction(ins_asm_fcmpe_s, -1, Rs1, Rs2, -1, -1, now_func);//�Ƚϸ�����
				int new_label = ++tot_label;//����һ���µ�label
				/*��δ����fcsr��־λ��˵������nan,��ֵRdΪ0*/
				get_new_asm_instruction(ins_asm_bgtz, fcsr, -1, -1, -1, new_label, now_func);
				/*����nan���*/
				get_new_asm_instruction(ins_asm_li, Rd, -1, -1, 1, -1, now_func);
				/*�����nan���*/
				get_new_asm_instruction(ins_asm_label, -1, -1, -1, -1, new_label, now_func);
				get_new_asm_instruction(ins_asm_li, Rd, -1, -1, 0, -1, now_func);
				break;
			}
		}
	}
	/*�ָ�ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, vRs1, fRs1, Rs1, vRs2, fRs2, Rs2);
}

void gen_branch(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd;
	int Rd;
	bool fRd = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	if (now_ins->branch_flag)//������ת
	{
		int L1 = now_ins->L1, L2 = now_ins->L2;
		/*ȷ������Ĵ���*/
		find_physical_reg(0, now_ins->num, vRd, t0, Rd, fRd);
		/*�ݴ��Լ�װ��*/
		pre_load_store(now_func, vRd, fRd, Rd);
		get_new_asm_instruction(ins_asm_bne, Rd, zero, -1, -1, L1, now_func);//����Ϊ����ת
		get_new_asm_instruction(ins_asm_j, -1, -1, -1, -1, L2, now_func);//����Ϊ����ת
		/*�ָ�ԭ��ֵ*/
		suf_load_store(now_func, vRd, fRd, Rd);
	}
	else//��������ת
	{
		int L1 = now_ins->L1;
		get_new_asm_instruction(ins_asm_j, -1, -1, -1, -1, L1, now_func);
	}
}

void gen_call(instruction* now_ins, functions* now_func)
{
	save_caller_saved_regs(now_func);
	int tot = 0, imm = 0;
	int vRd = now_ins->Rd;
	int Rd, Rs1;
	int op_rd = now_ins->tRd;
	bool fRd = false, fRs1 = false;
	if (now_ins->type_ret != ret_void)
	{
		find_physical_reg((op_rd == float32) ? 1 : 0, now_ins->num, vRd, (op_rd == float32) ? ft0 : t0, Rd, fRd);
		pre_load_store(now_func, vRd, fRd, Rd);
	}
	else
		vRd = Rd = -1;
	fRs1 = find_free_reg(Rs1, now_ins->num, 1, Rd);//���ҵ���ʱ����Ĵ���
	for (int i = 0; i < now_ins->formal_num.size(); ++i)
	{
		int op = now_ins->formal_type[i];//�β�����
		if (now_ins->formal_is_imm[i] == true)//�β���������
		{
			int val = now_ins->formal_num[i];
			if (op == float32)
			{
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, val, -1, now_func);
				if (i <= 7)
					get_new_asm_instruction(ins_asm_fmv_w_x, a0 + num_registers + i, ra, -1, -1, -1, now_func);
				else
				{
					/*�ݴ漰װ��*/
					pre_load_store(now_func, -1, fRs1, Rs1);
					get_new_asm_instruction(ins_asm_fmv_w_x, Rs1, ra, -1, -1, -1, now_func);
					get_new_asm_instruction(ins_asm_fsw, Rs1, sp, -1, imm, -1, now_func);
					imm += 4;
					/*�ָ�ԭ��ֵ*/
					suf_load_store(now_func, -1, fRs1, Rs1);
				}
			}
			else
			{
				if (i <= 7)
					get_new_asm_instruction(ins_asm_li, a0 + i, -1, -1, val, -1, now_func);
				else
				{
					get_new_asm_instruction(ins_asm_li, ra, -1, -1, val, -1, now_func);
					if (op == i32)
					{
						get_new_asm_instruction(ins_asm_sw, ra, sp, -1, imm, -1, now_func);
						imm += 4;
					}
					else if (op == i64)
					{
						get_new_asm_instruction(ins_asm_sd, ra, sp, -1, imm, -1, now_func);
						imm += 8;
					}
				}
			}
		}
		else//�β�������Ĵ���
		{
			int vRs2 = now_ins->formal_num[i];
			int Rs2;
			bool fRs2 = false;
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs2, (op == float32) ? ft2 : t2, Rs2, fRs2, 0, Rd, Rs1);
			/*�ݴ漰װ��*/
			pre_load_store(now_func, vRs2, fRs2, Rs2);
			if (op == float32)
			{
				if (i <= 7)
					get_new_asm_instruction(ins_asm_fadd_s, a0 + num_registers + i, Rs2, zero, -1, -1, now_func);
				else
				{
					get_new_asm_instruction(ins_asm_fsw, Rs2, sp, -1, imm, -1, now_func);
					imm += 4;
				}
			}
			else
			{
				if (i <= 7)
					get_new_asm_instruction(ins_asm_mv, a0 + i, Rs2, -1, -1, -1, now_func);
				else
				{
					if (op == i32)
					{
						get_new_asm_instruction(ins_asm_sw, Rs2, sp, -1, imm, -1, now_func);
						imm += 4;
					}
					else if (op == i64)
					{
						get_new_asm_instruction(ins_asm_sd, Rs2, sp, -1, imm, -1, now_func);
						imm += 8;
					}
				}
			}
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRs2, fRs2, Rs2);
		}
	}
	get_new_asm_instruction(ins_asm_call, -1, -1, -1, -1, map_function[now_ins->name], now_func);
	if (now_ins->type_ret != ret_void)
	{
		if (op_rd == float32)
			get_new_asm_instruction(ins_asm_fadd_s, Rd, fa0, zero, -1, -1, now_func);
		else
			get_new_asm_instruction(ins_asm_mv, Rd, a0, -1, -1, -1, now_func);
		suf_load_store(now_func, vRd, fRd, Rd);
	}
	load_caller_saved_regs(now_func, Rd);
}

void gen_ret(instruction* now_ins, functions* now_func)
{
	int vRd = -1, vRs = now_ins->Rs1;
	int op = (now_ins->tRs1 == float32) ? 1 : 0;
	int Rd = (op) ? fa0 + num_registers : a0, Rs;
	int imm;
	bool fRd = true, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	if (now_ins->type_ret!=ret_void)//������������void,������ֵ����a0/fa0
	{
		if (now_ins->fimm)//������
		{
			imm = now_ins->imm;
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd);
			if (op == float32)
			{
				get_new_asm_instruction(ins_asm_li, ra, -1, -1, imm, -1, now_func);
				get_new_asm_instruction(ins_asm_fmv_w_x, Rd, ra, -1, -1, -1, now_func);
			}
			else
			{
				get_new_asm_instruction(ins_asm_li, Rd, -1, -1, imm, -1, now_func);
			}
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd);
		}
		else
		{
			/*ȷ������Ĵ���*/
			find_physical_reg((op == float32) ? 1 : 0, now_ins->num, vRs, (op == float32) ? ft1 : t1, Rs, fRs);
			/*�ݴ��Լ�װ��*/
			pre_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
			if (op)
				get_new_asm_instruction(ins_asm_fadd_s, Rd, Rs, zero, -1, -1, now_func);//��Rs��ֵ����Rd
			else
				get_new_asm_instruction(ins_asm_mv, Rd, Rs, -1, -1, -1, now_func);//��Rs��ֵ����Rd
			/*�ָ�ԭ��ֵ*/
			suf_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
		}
	}
}

void gen_label(instruction* now_ins, functions* now_func)
{
	get_new_asm_instruction(ins_asm_label, -1, -1, -1, -1, now_ins->L1, now_func);
}

void gen_xtoy(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs = now_ins->Rs1;
	int op_Rd = (now_ins->tRd == float32) ? 1 : 0;
	int op_Rs = (now_ins->tRs1 == float32) ? 1 : 0;
	int Rd, Rs;
	bool fRd = true, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	/*ȷ������Ĵ���*/
	find_physical_reg(op_Rd, now_ins->num, vRd, (op_Rd) ? ft0 : t0, Rd, fRd);
	find_physical_reg(op_Rs, now_ins->num, vRs, (op_Rd) ? ft1 : t1, Rs, fRs, 0, Rd);
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
	if (now_ins->op == ins_fptosi)
	{
		get_new_asm_instruction(ins_asm_fcvt_w_s, Rd, Rs, -1, -1, -1, now_func);
	}
	else if (now_ins->op == ins_sitofp)
	{
		get_new_asm_instruction(ins_asm_fcvt_s_w, Rd, Rs, -1, -1, -1, now_func);
	}
	/*�ָ�ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
}

void gen_copy(instruction* now_ins, functions* now_func)
{
	int vRd = now_ins->Rd, vRs = now_ins->Rs1;
	int op = (now_ins->tRd == float32) ? 1 : 0;
	int Rd, Rs;
	bool fRd = true, fRs = false;//�Ƿ���Ҫռ���ѱ�ռ�õļĴ���
	/*ȷ������Ĵ���*/
	find_physical_reg(op, now_ins->num, vRd, (op) ? ft0 : t0, Rd, fRd);
	find_physical_reg(op, now_ins->num, vRs, (op) ? ft1 : t1, Rs, fRs, 0, Rd);
	/*�ݴ��Լ�װ��*/
	pre_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
	if (op)
		get_new_asm_instruction(ins_asm_fadd_s, Rd, Rs, zero, -1, -1, now_func);
	else
		get_new_asm_instruction(ins_asm_mv, Rd, Rs, -1, -1, -1, now_func);
	/*�ָ�ԭ��ֵ*/
	suf_load_store(now_func, vRd, fRd, Rd, vRs, fRs, Rs);
}

void gen_instruction(instruction* now_ins, functions* now_func)
{
	switch (now_ins->op)
	{
		case ins_global://global���
		{
			break;
		}
		case ins_load://load���
		{
			gen_load(now_ins, now_func);
			break;
		}
		case ins_store://store���
		{
			gen_store(now_ins, now_func);
			break;
		}
		case ins_alloca://alloca���
		{
			gen_alloca(now_ins, now_func);
			break;
		}
		case ins_getelementptr://GEP���
		{
			gen_GEP(now_ins, now_func);
			break;
		}
		case ins_add://add���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_fadd://fadd���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_sub://sub���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_fsub://fsub���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_mul://mul���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_fmul://fmul���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_sdiv://sdiv���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_fdiv://fdiv���
		{
			gen_operation(now_ins, now_func);
			break;

		}
		case ins_srem://srem���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_frem://frem���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_and://and���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_or://or���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_xor://xor���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_fneg://fneg���
		{
			gen_operation(now_ins, now_func);
			break;
		}
		case ins_icmp://icmp���
		{
			gen_xcmp(now_ins, now_func);
			break;
		}
		case ins_fcmp://fcmp���
		{
			gen_xcmp(now_ins, now_func);
			break;
		}
		case ins_br://br���
		{
			gen_branch(now_ins, now_func);
			break;
		}
		case ins_define:
		{
			break;
		}
		case ins_call://call���
		{
			gen_call(now_ins, now_func);
			break;
		}
		case ins_ret://ret���
		{
			gen_ret(now_ins, now_func);
			break;
		}
		case ins_label:
		{
			gen_label(now_ins, now_func);
			break;
		}
		case ins_unreachable://unreachable���
		{
			//gen_unreachable(now_ins, now_func);
			break;
		}
		case ins_sitofp://sitofp���
		{
			gen_xtoy(now_ins, now_func);
			break;
		}
		case ins_fptosi://fptosi���
		{
			gen_xtoy(now_ins, now_func);
			break;
		}
		case ins_copy:
		{
			gen_copy(now_ins, now_func);
			break;
		}
		case ins_sext:
		{
			gen_copy(now_ins, now_func);
			break;
		}
	}
}

/*���ɺ�����ʼָ��*/
void gen_func_start(functions* now_func)
{
	int size = now_func->size;
	/*�ı�sp*/
	if (is_in_range(size))
		get_new_asm_instruction(ins_asm_addi, sp, sp, -1, -size, -1, now_func);
	else
	{
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, size, -1, now_func);
		get_new_asm_instruction(ins_asm_sub, sp, sp, ra, -1, -1, now_func);
	}
	/*save callee_saved reg*/
	save_callee_saved_regs(now_func);
	/*raѹջ*/
	get_new_asm_instruction(ins_asm_sd, ra, sp, -1, size - 8, -1, now_func);
	/*s0ѹջ*/
	get_new_asm_instruction(ins_asm_sd, s0, sp, -1, size - 16, -1, now_func);
	/*s0��caller��ջ��*/
	if (is_in_range(size))
		get_new_asm_instruction(ins_asm_addi, s0, sp, -1, size, -1, now_func);
	else
	{
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, size, -1, now_func);
		get_new_asm_instruction(ins_asm_add, s0, sp, ra, -1, -1, now_func);
	}
	/*ȡ����*/
	Register_virtual* formal_arg = now_func->reg_head;
	for (int i = 0; i < now_func->args.size(); ++i)
	{
		formal_arg = formal_arg->next;
		int vRd = formal_arg->num, Rd;
		int op = now_func->args[i];
		int imm = 0;
		bool fRd = false;
		/*ȷ������Ĵ���*/
		find_physical_reg((op == float32) ? 1 : 0, now_func->ins_start, vRd, (op == float32) ? ft0 : t0, Rd, fRd, 0, s0);
		/*�ݴ��Լ�װ��*/
		pre_load_store(now_func, vRd, fRd, Rd);
		{
			if (op == float32)
			{
				if (i <= 7)
					get_new_asm_instruction(ins_asm_fadd_s, Rd, a0 + num_registers + i, zero, -1, -1, now_func);
				else
				{
					get_new_asm_instruction(ins_asm_flw, Rd, s0, -1, imm, -1, now_func);
					imm += 4;
				}
			}
			else
			{
				if (i <= 7)
				{
					get_new_asm_instruction(ins_asm_mv, Rd, a0 + i, -1, -1, -1, now_func);
				}
				else
				{
					if (op == i32)
					{
						get_new_asm_instruction(ins_asm_lw, Rd, s0, -1, imm, -1, now_func);
						imm += 4;
					}
					else if (op == i64)
					{
						get_new_asm_instruction(ins_asm_ld, Rd, s0, -1, imm, -1, now_func);
						imm += 8;
					}
				}
			}
		}
		/*�ָ�ԭ��ֵ*/
		suf_load_store(now_func, vRd, fRd, Rd);
	}
}

/*���ɺ�������ָ��*/
void gen_func_end(functions* now_func)
{
	int size = now_func->size;
	/*load callee_saved regs*/
	load_callee_saved_regs(now_func);
	/*�ָ�ra*/
	get_new_asm_instruction(ins_asm_ld, ra, sp, -1, size - 8, -1, now_func);
	/*�ָ�s0*/
	get_new_asm_instruction(ins_asm_ld, s0, sp, -1, size - 16, -1, now_func);
	/*�ָ�sp*/
	if (is_in_range(size))
		get_new_asm_instruction(ins_asm_addi, sp, sp, -1, size, -1, now_func);
	else
	{
		get_new_asm_instruction(ins_asm_li, ra, -1, -1, size, -1, now_func);
		get_new_asm_instruction(ins_asm_add, sp, sp, ra, -1, -1, now_func);
	}
	/*��ת��ԭָ��*/
	get_new_asm_instruction(ins_asm_jr, -1, ra, -1, -1, -1, now_func);
}

void get_asm()
{
	clear_occupied_reg();
	/*���ó������*/
	printf("    .text\n");
	//printf("    .global %s\n", ("F" + std::to_string(num_start_func)).c_str());
	printf("    .global _start\n");
	/*����ȫ�ֱ���*/
	variable_table* global_head = global;
	while (global_head->next != NULL)
	{
		global_head = global_head->next;
		printf("%s:\n", ("G" + std::to_string(global_head->num_reg)).c_str());
		for (auto it : global_head->val)
			printf("    .word %u\n", it);
		if (global_head->cnt - global_head->val.size() > 0)
			printf("    .zero %d\n", (int)(global_head->cnt - global_head->val.size()) << 2);
	}
	/*���ں����ڵ�ÿ��������ɻ�����*/
	functions* func = func_head;
	while (func->next != NULL)
	{
		func = func->next;
		if (func->num == num_start_func)
			printf("_start:\n");
		else
			printf("%s:\n", ("F" + std::to_string(func->num)).c_str());
		gen_func_start(func);
		instruction* ins_head = func->ins_head;
		while (ins_head->next != NULL)
		{
			ins_head = ins_head->next;
			gen_instruction(ins_head, func);
			if (ins_head->op == ins_icmp)
				ins_head = ins_head->next;
			if (ins_head == NULL)
				break;
		}
		gen_func_end(func);
		peephole(func);//�����Ż�
		print_asm(func);//ָ�����
	}
}