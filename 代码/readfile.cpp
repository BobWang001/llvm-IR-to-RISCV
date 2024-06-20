#include "riscv.h"

extern map<int, Register_virtual*>map_global_register_position;//全局寄存器编号到指针的映射
extern map<int, Register_virtual*>map_local_register_position;//局部虚拟寄存器编号到指针的映射
extern map<int, int>map_register_local;//寄存器编号到变量编号的映射
extern map<int, int>map_register_local_alloca;//alloca语句内寄存器编号到变量编号的映射
extern int total_global;//存储变量的数量
extern variable_table* global, * global_tail;
extern int tot_instructions;//总的指令数
extern map<std::string, int>cond_num;
extern map<type_label, int>label_num;
extern map<int, int>label_ins;//label到指令的映射
extern int tot_label;
extern map<int, instruction*>map_instruction_position;//指令编号到指针的映射
extern map<int, variable_table*>map_variable_position;//变量编号到指针的映射

unsigned int floatToBinary(float num)//将浮点数转换为对应的二进制数
{
	// 使用union来访问float的位表示
	union
	{
		float f;
		unsigned int i;
	}b;
	b.f = num;
	std::bitset<32> bs(b.i); // 创建一个32位的bitset
	return bs.to_ulong();
}

/*在该函数的指令末尾插入一条指令*/
void insert_instruction(functions* num_func,instruction* new_instruction)
{
	map_instruction_position[new_instruction->num] = new_instruction;
	num_func->ins_tail->next = new_instruction;
	num_func->ins_tail = new_instruction;
}

/*新申请一个局部虚拟寄存器*/
void get_new_register(Register_virtual* new_register, type_label name, functions* num_func)
{
	new_register->name = name;
	new_register->num = ++total_register;
	num_func->map_local_register[name] = new_register->num;
	num_func->map_local_register_position[new_register->num] = new_register;
	map_local_register_position[new_register->num] = new_register;
	num_func->reg_tail->next = new_register;
	num_func->reg_tail = new_register;
}

void new_variable_type(variable_table* new_global, std::string word)
{
	int len = word.length();
	for (int p = 0; p < len; )
	{
		if (word[p] == '[' || word[p] == ']' || word[p] == 'x' || word[p] == ' ' || word[p] == ',')
		{
			p++;
			continue;
		}
		std::string ret;
		ret.push_back(word[p]);
		p++;
		while (word[p] != ' ' && word[p] != ']')
		{
			ret.push_back(word[p]);
			p++;
		}
		if (ret == "i32")
			new_global->type = 0;
		else if (ret == "float")
			new_global->type = 1;
		else
		{
			int val = atoi(ret.c_str());//该维度的大小
			new_global->dim++;
			new_global->size.push_back(val);
			new_global->cnt *= val;
		}
		p++;
	}
}

void new_variable_value(variable_table* new_global, std::string word)
{
	int len = word.length();
	for (int p = 0; p < len; )
	{
		if (word[p] == '[' || word[p] == ']' || word[p] == 'x' || word[p] == ' ' || word[p] == ',')
		{
			p++;
			continue;
		}
		std::string ret;
		ret.push_back(word[p]);
		p++;
		while (word[p] != ' ' && word[p] != ']')
		{
			ret.push_back(word[p]);
			p++;
		}
		if (ret == "i32" || ret == "float")
		{
			bool find_num = 0, find_brack = 0;
			std::string num;
			while ((word[p] < '0' || word[p] > '9') && word[p] != '-')
			{
				if (word[p] == ']')
				{
					p++;
					find_brack = 1;
					break;
				}
				else if (word[p] == ' ')
					p++;
			}
			if (find_brack)
				continue;
			find_num = 1;//找到一个值
			while ((word[p] >= '0' && word[p] <= '9') || word[p] == '.' || word[p]=='-')
			{
				num.push_back(word[p]);
				p++;
			}
			if (!find_num)
				continue;
			if (new_global->type == 0)
				new_global->val.push_back(atoi(num.c_str()));
			else
				new_global->val.push_back(floatToBinary(atof(num.c_str())));
		}
		p++;
	}
}

void new_variable(int op, std::string line, functions* num_func = NULL)
{

	printf(";%s\n", line.c_str());

	variable_table* new_variable = new variable_table;
	Register_virtual* new_register = new Register_virtual;
	new_register->used = 0;
	new_register->reg_phisical = -1;
	bool is_ins = 0, name = 0, size_type = 0, val = 0;//标记是否已经找到了全局变量的名称，大小类型，初始值
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == 9 || line[p] == ',' || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		word.push_back(line[p]);
		switch (line[p])
		{
			case '[': 
			{
				int top = 1;
				while (top)
				{
					p++;
					word.push_back(line[p]);
					if (line[p] == '[')
						top++;
					else if (line[p] == ']')
						top--;
				}
				if (line[p] == ']')
					p++;
				break;
			}
			default:
			{
				p++;
				while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
				{
					word.push_back(line[p]);
					p++;
				}
				break;
			}
		}
		if (word == "global" || word == "alloca")
			continue;
		if (word == "=")
			continue;
		if (!name)
		{
			new_variable->name = word;
			new_register->name = word;
			name = 1;
		}
		else if (!size_type)
		{
			if (word[0] != '[')
				new_variable->type = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			else
				new_variable_type(new_variable, word);
			new_register->type = i64;//寄存器存的64位地址
			size_type = 1;
		}
		else if (!val)
		{
			if (word[0] != '[')
			{
				if (new_variable->type == 0)//整数直接赋值
					new_variable->val.push_back(atoi(word.c_str()));
				else//浮点数需要转换一下
					new_variable->val.push_back(floatToBinary(atof(word.c_str())));
			}
			else
				new_variable_value(new_variable, word);
			val = 1;
		}
	}
	if (!is_ins)
		return;
	new_variable->num = total_global + 1;//记录编号
	new_register->num = ++total_register;
	map_register_local_alloca[new_register->num] = new_variable->num;
	if (op)
	{
		num_func->map_local[new_variable->name] = new_variable->num;
		num_func->map_local_register[new_register->name] = new_register->num;
		num_func->map_local_register_position[new_register->num] = new_register;
		map_local_register_position[new_register->num] = new_register;
		/*插入alloca指令*/
		instruction* new_alloca = new instruction;
		new_alloca->num = ++tot_instructions;
		new_alloca->op = 3;
		new_alloca->Rd = new_register->num;
		new_alloca->tRd = new_register->type;
		insert_instruction(num_func, new_alloca);
	}
	else
	{
		map_global[new_variable->name] = new_variable->num;
		map_global_register[new_register->name] = new_register->num;
		map_global_register_position[new_register->num] = new_register;
	}
	total_global += new_variable->cnt;
	new_variable->next = NULL;
	map_variable_position[new_variable->num] = new_variable;
	//加入局部变量
	if (op)
	{
		num_func->total_actual += new_variable->cnt;
		num_func->local_tail->next = new_variable;
		num_func->local_tail = new_variable;
		num_func->reg_tail->next = new_register;
		num_func->reg_tail = new_register;
	}
	else
	{
		global_tail->next = new_variable;
		global_tail = new_variable;
	}

	printf(";register_name=%s,number=%d,type=%d\n", new_register->name.c_str(), new_register->num, new_register->type);
	printf(";variable_name=%s,number=%d,type=%d,dim=%d,cnt=%d\n", new_variable->name.c_str(), new_variable->num, new_variable->type, new_variable->dim, new_variable->cnt);
	printf(";size: ");
	for (auto it : new_variable->size)
		printf("%u ", it);
	printf("\n;val: ");
	for (auto it : new_variable->val)
		printf("%u ", it);
	printf("\n\n");

}


vector<int> get_size(std::string name, functions* num_func)
{
	vector<int>ret;
	if (map_global.count(name) != 0)
	{
		variable_table* head = global;
		while (head->next != NULL)
		{
			head = head->next;
			if (head->name == name)
			{
				for (auto it : head->size)
					ret.push_back(it);
				return ret;
			}
		}
	}
	else
	{
		variable_table* head = num_func->local_head;
		while (head->next != NULL)
		{
			head = head->next;
			if (head->name == name)
			{
				for (auto it : head->size)
					ret.push_back(it);
				return ret;
			}
		}
	}
	return ret;
}

void get_register_imm(instruction* new_load, std::string line, functions* num_func)
{
	bool fRs = 0;//标记是否已经找到了源变量
	int len = line.length();
	vector<int>size, size_gep;
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == 9 || line[p] == '*' || line[p]==',' || line[p] == '(' || line[p] == ')')
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		std::string word;
		word.push_back(line[p]);
		bool bracket = 0;
		switch (line[p])
		{
			case '[':
			{
				bracket = 1;
				int top = 1;
				while (top)
				{
					p++;
					if (line[p] == '[')
						top++;
					else if (line[p] == ']')
						top--;
				}
				if (line[p] == ']')
					p++;
				break;
			}
			default:
			{
				p++;
				while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
				{
					word.push_back(line[p]);
					p++;
				}
				break;
			}
		}
		if (bracket)
			continue;
		if (word == "=" || word == "getelementptr")
			continue;
		if (!fRs)
		{
			if (map_global_register.count(word) != 0)
				new_load->Rs1 = map_global_register[word];
			else
				new_load->Rs1 = num_func->map_local_register[word];
			size = get_size(word, num_func);
			fRs = 1;
		}
		else
		{
			if (word[0] >= '0' && word[0] <= '9')
				size_gep.push_back(atoi(word.c_str()));
		}
	}
	int imm = 0, res = 1;
	for (auto it : size)
		res *= it;
	int nw = 1;
	for (auto it : size)
	{
		res /= it;
		imm += res * size_gep[nw];
		nw++;
	}
	new_load->fimm = 1;
	new_load->imm = imm;
	size_gep.clear(); size_gep.shrink_to_fit();
	size.clear(); size.shrink_to_fit();
}

void new_load(int op,std::string line,functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_load = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs = 0, tRs = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "load")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_load->Rd = new_register->num;
			fRd = 1;
		}
		else if (!tRd)
		{
			new_load->tRd = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			new_register->type = new_load->tRd;
			tRd = 1;
		}
		else if (!tRs)
		{
			new_load->tRs1 = ((word == "i32*") ? 0 : ((word == "float*") ? 1 : 2));
			tRs = 1;
		}
		else if (!fRs)
		{

			printf("%s\n", word.c_str());

			if (word == "getelementptr")
			{
				std:string ret = line.substr(p+1);
				get_register_imm(new_load, ret, num_func);
			}
			else
			{
				if (map_global_register.count(word) != 0)
					new_load->Rs1 = map_global_register[word];
				else
					new_load->Rs1 = num_func->map_local_register[word];
			}
			fRs = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_load->num = ++tot_instructions;
	new_load->op = op;
	insert_instruction(num_func, new_load);

	printf(";Rd=%d type=%d Rs=%d type=%d ", new_load->Rd, new_load->tRd, new_load->Rs1, new_load->tRs1);
	if (new_load->fimm)
		printf("imm=%d", new_load->imm);
	printf("\n\n");

}

void new_store(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_store = new instruction;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs_imm = 0, tRs_imm = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "store")
			continue;
		if (!tRs_imm)
		{
			new_store->tRs1 = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			tRs_imm = 1;
		}
		else if (!fRs_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_store->fimm = 1;
				if (new_store->tRs1 == 0)
					new_store->imm = atoi(word.c_str());
				else
					new_store->imm = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_store->fimm = 0;
				if (map_global_register.count(word) != 0)
					new_store->Rs1 = map_global_register[word];
				else
					new_store->Rs1 = num_func->map_local_register[word];
			}
			fRs_imm = 1;
		}
		else if (!tRd)
		{
			new_store->tRd = ((word == "i32*") ? 0 : ((word == "float*") ? 1 : 2));
			tRd = 1;
		}
		else if (!fRd)
		{
			if (map_global_register.count(word) != 0)
				new_store->Rd = map_global_register[word];
			else
				new_store->Rd = num_func->map_local_register[word];
			fRd = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_store->num = ++tot_instructions;
	new_store->op = op;
	insert_instruction(num_func, new_store);

	printf(";Rd=%d type=%d ", new_store->Rd, new_store->tRd);
	if (new_store->fimm)
		printf("imm=%u ", new_store->imm);
	else printf("Rs=%d ", new_store->Rs1);
	printf("type=%d\n", new_store->tRs1);
	printf("\n");

}

void new_GEP(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_gep = new instruction;
	bool is_ins = 0, fRd = 0, fRs = 0;//标记是否已经找到了目的变量，源变量
	int len = line.length();
	Register_virtual* new_register = new Register_virtual;
	vector<int>size, size_gep;
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == 9 || line[p] == '*' || line[p] == ','
			|| line[p] == '(' || line[p] == ')' || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		word.push_back(line[p]);
		bool bracket = 0;
		switch (line[p])
		{
			case '[':
			{
				bracket = 1;
				int top = 1;
				while (top)
				{
					p++;
					if (line[p] == '[')
						top++;
					else if (line[p] == ']')
						top--;
				}
				if (line[p] == ']')
					p++;
				break;
			}
			default:
			{
				p++;
				while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
				{
					word.push_back(line[p]);
					p++;
				}
				break;
			}
		}
		if (bracket)
			continue;
		if (word == "=" || word == "getelementptr")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_gep->Rd = new_register->num;
			fRd = 1;
		}
		else if (!fRs)
		{
			if (map_global_register.count(word) != 0)
			{
				new_gep->Rs1 = map_global_register[word];
				if (new_register != NULL)
					new_register->type = map_global_register_position[new_gep->Rs1]->type;
			}
			else
			{
				new_gep->Rs1 = num_func->map_local_register[word];
				new_register->type = num_func->map_local_register_position[new_gep->Rs1]->type;
			}
			size = get_size(word, num_func);
			fRs = 1;
		}
		else
		{
			if (word[0] >= '0' && word[0] <= '9')
				size_gep.push_back(atoi(word.c_str()));
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_gep->num = ++tot_instructions;
	new_gep->op = op;
	int imm = 0, res = 1;
	for (auto it : size)
		res *= it;
	int nw = 1;
	for (auto it : size)
	{
		res /= it;
		imm += res * size_gep[nw];
		nw++;
	}
	new_gep->imm = imm;
	insert_instruction(num_func, new_gep);

	printf(";Rd=%d Rs=%d imm=%d\n", new_gep->Rd, new_gep->Rs1, new_gep->imm);
	printf("\n");

	size_gep.clear(); size_gep.shrink_to_fit();
	size.clear(); size.shrink_to_fit();

}

void new_operation(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_operation = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, fRs1_imm = 0, fRs2_imm = 0, type = 0;//标记是否已经找到了目的变量及其type，源变量(1/2)及其type
	int len = line.length();
	if (op == 15)
		fRs2_imm = 1;
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "add" || word == "fadd" || word == "sub" || word == "fsub"
			|| word == "mul" || word == "fmul" || word == "sdiv" || word == "fdiv"
			|| word == "and" || word == "or" || word == "xor"
			|| word == "srem" || word == "frem" || word == "fneg")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_operation->Rd = new_register->num;
			fRd = 1;
		}
		else if (!type)
		{
			new_register->type = new_operation->tRd = new_operation->tRs1 = new_operation->tRs2 = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			type = 1;
		}
		else if (!fRs1_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_operation->fimm1 = 1;
				if (new_operation->tRs1 != float32)
					new_operation->imm1 = atoi(word.c_str());
				else
					new_operation->imm1 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_operation->fimm1 = 0;
				if (map_global_register.count(word) != 0)
					new_operation->Rs1 = map_global_register[word];
				else
					new_operation->Rs1 = num_func->map_local_register[word];
			}
			fRs1_imm = 1;
		}
		else if (!fRs2_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_operation->fimm2 = 1;
				if (new_operation->tRs2 == 0)
					new_operation->imm2 = atoi(word.c_str());
				else
					new_operation->imm2 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_operation->fimm2 = 0;
				if (map_global_register.count(word) != 0)
					new_operation->Rs2 = map_global_register[word];
				else
					new_operation->Rs2 = num_func->map_local_register[word];
			}
			fRs2_imm = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_operation->num = ++tot_instructions;
	new_operation->op = op;
	insert_instruction(num_func, new_operation);

	printf(";Rd=%d ", new_operation->Rd);
	if (new_operation->fimm1)
		printf("imm1=%u ", new_operation->imm1);
	else printf("Rs1=%d ", new_operation->Rs1);
	if (new_operation->fimm2)
		printf("imm2=%u ", new_operation->imm2);
	else printf("Rs2=%d ", new_operation->Rs2);
	printf("type=%d\n", new_operation->tRd);
	printf("\n");
}

void new_xcmp(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_xcmp = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, cond = 0, fRs1_imm = 0, fRs2_imm = 0, type = 0;//标记是否已经找到了目的变量及其type，源变量(1/2)及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "icmp" || word == "fcmp")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_xcmp->Rd = new_register->num;
			fRd = 1;
		}
		else if (!cond)
		{
			new_xcmp->cond = cond_num[word];
			cond = 1;
		}
		else if (!type)
		{
			new_register->type = new_xcmp->tRd = new_xcmp->tRs1 = new_xcmp->tRs2 = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			type = 1;
		}
		else if (!fRs1_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_xcmp->fimm1 = 1;
				if (new_xcmp->tRs1 != float32)
					new_xcmp->imm1 = atoi(word.c_str());
				else
					new_xcmp->imm1 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_xcmp->fimm1 = 0;
				if (map_global_register.count(word) != 0)
					new_xcmp->Rs1 = map_global_register[word];
				else
					new_xcmp->Rs1 = num_func->map_local_register[word];
			}
			fRs1_imm = 1;
		}
		else if (!fRs2_imm)
		{
			if ((word[0] >= '0' && word[0] <= '9') || word[0] == '-') //存在立即数
			{
				new_xcmp->fimm2 = 1;
				if (new_xcmp->tRs2 == 0)
					new_xcmp->imm2 = atoi(word.c_str());
				else
					new_xcmp->imm2 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_xcmp->fimm2 = 0;
				if (map_global_register.count(word) != 0)
					new_xcmp->Rs2 = map_global_register[word];
				else
					new_xcmp->Rs2 = num_func->map_local_register[word];
			}
			fRs2_imm = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_xcmp->num = ++tot_instructions;
	new_xcmp->op = op;
	insert_instruction(num_func, new_xcmp);

	printf(";Rd=%d ", new_xcmp->Rd);
	if (new_xcmp->fimm1)
		printf("imm1=%u ", new_xcmp->imm1);
	else printf("Rs1=%d ", new_xcmp->Rs1);
	if (new_xcmp->fimm2)
		printf("imm2=%u ", new_xcmp->imm2);
	else printf("Rs2=%d ", new_xcmp->Rs2);
	printf("cond=%d type=%d\n", new_xcmp->cond, new_xcmp->tRd);
	printf("\n");
}

void new_branch(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_branch = new instruction;
	bool is_ins = 0, cond = 0, label1 = 0, label2 = 0;//标记是否已经找到了转移条件，为真转移目的地，为假转移目的地
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "br")
			continue;
		if (word == "i1")
		{
			new_branch->branch_flag = 1;//条件跳转
			continue;
		}
		if (word == "label")
		{
			if (!new_branch->branch_flag)
			{
				cond = label2 = 1;
			}
			continue;
		}
		if (!cond)
		{
			if (map_global_register.count(word) != 0)
				new_branch->Rs1 = map_global_register[word];
			else
				new_branch->Rs1 = num_func->map_local_register[word];
			cond = 1;
		}
		else if (!label1)
		{
			new_branch->L1 = label_num[word];
			label1 = 1;
		}
		else if (!label2)
		{
			new_branch->L2 = label_num[word];
			label2 = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_branch->num = ++tot_instructions;
	new_branch->op = op;
	insert_instruction(num_func, new_branch);

	printf(";contion=%d ", (new_branch->branch_flag) ? 1 : 0);
	if (new_branch->branch_flag)
		printf("i1=%d ", new_branch->Rs1);
	printf("label1=%d ", new_branch->L1);
	if (new_branch->branch_flag)
		printf("label2=%d ", new_branch->L2);
	printf("\n\n");

}

/*获取call指令的参数列表*/
void get_instruction_args(instruction* ins, std::string line, functions* num_func)
{
	int len = line.length();
	bool last_type = 0;//当前参数类型
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "i32" || word == "float")//新增一个参数
		{
			last_type = (word == "i32") ? 0 : 1;
			ins->formal_type.push_back(last_type);
			ins->tot_formal++;
		}
		else//参数名,新增一个局部变量
		{
			if ((word[0] >= '0' && word[0] <= '9') || word[0] == '-')//参数为立即数
			{
				ins->formal_is_imm.push_back(true);
				if (last_type == float32)//float
					ins->formal_num.push_back(floatToBinary(atof(word.c_str())));
				else//i32
					ins->formal_num.push_back(atoi(word.c_str()));
			}
			else
			{
				ins->formal_is_imm.push_back(false);
				if (map_global_register.count(word) != 0)
					ins->formal_num.push_back(map_global_register[word]);
				else ins->formal_num.push_back(num_func->map_local_register[word]);
			}
		}
	}
}

void new_call(int op, std::string line, functions* num_func)
{
	
	printf(";%s\n", line.c_str());

	instruction* new_call = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, tRd = 0, name = 0;//标记是否已经找到了目的变量及其type，被调函数名
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';' || line[p] == '(')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格或者'('停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ','
			&& line[p] != '(' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "call")
			continue;
		if (!fRd)
		{
			if (word == "void" || word == "i32" || word == "float")
			{
				new_call->type_ret = 2;
				tRd = 1;
			}
			else
			{
				get_new_register(new_register, word, num_func);
				new_call->Rd = new_register->num;
			}
			fRd = 1;
		}
		else if (!tRd)
		{
			new_register->type = new_call->type_ret = ((word == "i32") ? 0 : 1);
			tRd = 1;
		}
		else if (!name)
		{
			new_call->name = word;
			name = 1;
		}
	}
	for (int p = 0; p < len; )
	{
		while (line[p] != '(')
			p++;
		p++;
		std::string word;
		while (line[p] != ')')
		{
			word.push_back(line[p]);
			p++;
		}
		get_instruction_args(new_call, word, num_func);//获取参数列表
		break;
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_call->num = ++tot_instructions;
	new_call->op = op;
	num_func->max_formal = max(num_func->max_formal, new_call->tot_formal);
	insert_instruction(num_func, new_call);

	printf(";type=%d arg_cnt=%d ", new_call->type_ret, new_call->tot_formal);
	if (new_call->type_ret != 2)
		printf("ret_num=%d\n", new_call->Rd);
	else printf("\n");
	int nw = 0;
	for (auto it : new_call->formal_num)
	{
		if (new_call->formal_is_imm[nw])
			printf(";imm=%d type=%d\n", it, (new_call->formal_type[nw]) ? 1 : 0);
		else
			printf(";number=%d type=%d\n", it, (new_call->formal_type[nw]) ? 1 : 0);
		nw++;
	}
	printf("\n");

}

void new_ret(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_ret = new instruction;
	bool is_ins = 0, type = 0, tRs = 0;//标记是否已经找到返回类型，源变量
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "ret")
			continue;
		if (!type)
		{
			if (word == "void")
			{
				new_ret->type_ret = 2;
				break;
			}
			else new_ret->type_ret = ((word == "i32") ? 0 : 1);
			type = 1;
		}
		else if (!tRs)
		{
			if (map_global_register.count(word) != 0)
				new_ret->Rs1 = map_global_register[word];
			else new_ret->Rs1 = num_func->map_local_register[word];
			tRs = 1;
		}
	}
	if (!is_ins)
		return;
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_ret->num = ++tot_instructions;
	new_ret->op = op;
	insert_instruction(num_func, new_ret);

	printf(";type=%d ", new_ret->type_ret);
	if (new_ret->type_ret != 2)
		printf("number=%d\n", new_ret->Rs1);
	else printf("\n");
	printf("\n");

}

void get_args(functions* new_function, std::string line)
{
	int len = line.length();
	bool last_type = 0;//当前参数类型
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 13)
		{
			p++;
			continue;
		}
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "i32" || word == "float")//新增一个参数
		{
			new_function->total_formal++;
			new_function->args.push_back((word == "i32") ? 0 : 1);
			last_type = (word == "i32") ? 0 : 1;
		}
		else//参数名,新增一个局部寄存器
		{
			Register_virtual* new_register = new Register_virtual;
			get_new_register(new_register, word, new_function);
			new_register->type = last_type;
		}
	}
}

functions* new_function(std::string line)
{

	printf(";%s\n", line.c_str());

	functions* new_function = new functions;
	new_function->num = ++tot_functions;
	bool type = 0, name = 0, args = 0;//标记是否已经找到了函数的type，函数名以及参数
	int len = line.length();
	/*先找到除参数列表以外的信息*/
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';' || line[p] == '(')
			break;
		/*获得单词：除了存在小括号进行括号匹配以外，其他均为读到空格停止*/
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ','
			&& line[p] != '(' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "define")
			continue;
		if (!type)
		{
			if (word == "void")
				new_function->type = 2;
			else if (word == "i32")
				new_function->type = 0;
			else if (word == "float")
				new_function->type = 1;
			type = 1;
		}
		else if (!name)
		{
			new_function->name = word;
			map_function[word] = tot_functions;
			name = 1;
		}
	}
	for (int p = 0; p < len; )
	{
		while (line[p] != '(')
			p++;
		p++;
		std::string word;
		while (line[p] != ')')
		{
			word.push_back(line[p]);
			p++;
		}
		get_args(new_function, word);//获取参数列表
		break;
	}
	func_tail->next = new_function;
	func_tail = new_function;

	printf(";name=%s number=%d formal=%d\n", new_function->name.c_str(), new_function->num, new_function->total_formal);
	printf(";type: ");
	for (auto it : new_function->args)
	{
		if (it)
			printf("1 ");
		else printf("0 ");
	}
	printf("\n\n");

	return new_function;
}

void end_function(functions* now_function)
{
	now_function->tot_arg = now_function->total_actual + now_function->total_formal;
	//为局部变量分配空间,只写偏移量
	int imm = 16;
	variable_table* head = now_function->local_head;
	while (head->next != NULL)
	{
		head = head->next;
		imm += (head->cnt << 2);
		if(head->type==i64)
			imm += (head->cnt << 2);
		now_function->imm_local[head->num] = -imm;
	}
	imm += (max(now_function->max_formal - 8, 0) + 16) << 2;//计算栈帧大小,多留了24字节
	now_function->size = imm;
	head = now_function->local_head;
	while (head->next != NULL)
	{
		head = head->next;
		now_function->imm_local[head->num] += imm;
	}

	printf(";size=%d formal_num=%d max_num=%d\n", now_function->size, now_function->total_formal, now_function->max_formal);
	head = now_function->local_head;
	while (head->next != NULL)
	{
		head = head->next;
		printf(";name=%s num=%d imm=%d\n", head->name.c_str(), head->num, now_function->imm_local[head->num]);
	}
	printf("\n");

}

int check_label(std::string s)
{
	int len = s.length();
	for (int p = 0; p < len; ++p)
	{
		if (s[p] == ' ' || s[p] == 9 || s[p] == 13)
			continue;
		if (s[p] == ';')
			return 0;
		if (s[p] == ':')
			return 1;
	}
	return 0;
}

std::string get_label(std::string s)
{
	int len = s.length();
	std::string ret = "%";
	for (int p = 0; p < len;)
	{
		if (s[p] == ' ' || s[p] == 9 || s[p] == 13)
		{
			p++;
			continue;
		}
		while (s[p] != ':')
		{
			ret.push_back(s[p]);
			p++;
		}
		break;
	}
	return ret;
}

void new_label(int op, type_label label, functions* num_func)
{

	printf(";%s:\n", label.c_str());

	instruction* new_label = new instruction;
	new_label->num = ++tot_instructions;
	new_label->op = op;
	new_label->L1 = label_num[label];
	insert_instruction(num_func, new_label);
	label_ins[label_num[label]] = new_label->num;

	printf(";num=%d\n\n", new_label->L1);

}

std::string get_new_line(std::string line)
{
	int len = line.length();
	std::string ret;
	for (int p = 0; p < len; ++p)
	{
		if (line[p] == ':')
		{
			ret = line.substr(p + 1);
			break;
		}
	}
	return ret;
}

void new_unreachable(int op,std::string line, functions* num_func)
{

	printf(";%s:\n", line.c_str());

	instruction* new_unreachable = new instruction;
	new_unreachable->op = op;
	new_unreachable->num = ++tot_instructions;
	insert_instruction(num_func, new_unreachable);
}

void new_xtoy(int op,std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_xtoy = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs_imm = 0, tRs_imm = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "fptosi" || word == "sitofp" || word == "to")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_xtoy->Rd = new_register->num;
			fRd = 1;
		}
		else if (!tRs_imm)
		{
			new_xtoy->tRs1 = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			tRs_imm = 1;
		}
		else if (!fRs_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_xtoy->fimm1 = 1;
				if (new_xtoy->tRs1 != float32)
					new_xtoy->imm1 = atoi(word.c_str());
				else
					new_xtoy->imm1 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_xtoy->fimm1 = 0;
				if (map_global_register.count(word) != 0)
					new_xtoy->Rs1 = map_global_register[word];
				else
					new_xtoy->Rs1 = num_func->map_local_register[word];
			}
			fRs_imm = 1;
		}
		else if (!tRd)
		{
			new_register->type = new_xtoy->tRd = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			tRd = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_xtoy->num = ++tot_instructions;
	new_xtoy->op = op;
	insert_instruction(num_func, new_xtoy);

	printf(";Rd=%d type_Rd=%d ", new_xtoy->Rd, new_xtoy->tRd);
	if (new_xtoy->fimm1)
		printf("imm=%u ", new_xtoy->imm1);
	else printf("Rs=%d ", new_xtoy->Rs1);
	printf("type_Rs=%d\n", new_xtoy->tRs1);
	printf("\n");
}

void new_zext(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_zext = new instruction;
	Register_virtual* new_register = new Register_virtual;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs_imm = 0, tRs_imm = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9 || line[p] == 13)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != 13)
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "zext" || word == "to")
			continue;
		if (!fRd)
		{
			get_new_register(new_register, word, num_func);
			new_zext->Rd = new_register->num;
			fRd = 1;
		}
		else if (!tRs_imm)
		{
			new_zext->tRs1 = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			tRs_imm = 1;
		}
		else if (!fRs_imm)
		{
			if ((word[0] >= '0' && word[0] <= '9') || word[0] == '-')
			{
				new_zext->fimm1 = 1;
				if (new_zext->tRs1 != float32)
					new_zext->imm1 = atoi(word.c_str());
				else
					new_zext->imm1 = floatToBinary(atof(word.c_str()));

			}
			else
			{
				new_zext->fimm1 = 0;
				if (map_global_register.count(word) != 0)
					new_zext->Rs1 = map_global_register[word];
				else
					new_zext->Rs1 = num_func->map_local_register[word];
			}
			fRs_imm = 1;
		}
		else if (!tRd)
		{
			new_zext->tRd = ((word == "i32") ? 0 : ((word == "float") ? 1 : 2));
			new_register->type = new_zext->tRd;
			tRd = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_zext->num = ++tot_instructions;
	new_zext->op = op;
	insert_instruction(num_func, new_zext);

	printf(";Rd=%d type=%d ", new_zext->Rd, new_zext->tRd);
	if (new_zext->fimm)
	{
		printf("imm=%u type=%d", new_zext->imm, new_zext->tRs1);
	}
	else
	{
		printf("Rs=%d type=%d", new_zext->Rs1, new_zext->tRs1);
	}
	printf("\n\n");

}

void read(int option, std::string line, functions* num_func, bool in_func)//读入
{
	switch (option)
	{
		case ins_global://global语句
		{
			new_variable(0, line);//新建一个全局变量
			break;
		}
		case ins_load://load语句
		{
			new_load(option, line, num_func);
			break;
		}
		case ins_store://store语句
		{
			new_store(option, line, num_func);
			break;
		}
		case ins_alloca://alloca语句
		{
			new_variable(1, line, num_func);
			break;
		}
		case ins_getelementptr://GEP语句
		{
			new_GEP(option, line, num_func);
			break;
		}
		case ins_add://add语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_fadd://fadd语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_sub://sub语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_fsub://fsub语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_mul://mul语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_fmul://fmul语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_sdiv://sdiv语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_fdiv://fdiv语句
		{
			new_operation(option, line, num_func);
			break;
		
		}
		case ins_srem://srem语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_frem://frem语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_and://and语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_or://or语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_xor://xor语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_fneg://fneg语句
		{
			new_operation(option, line, num_func);
			break;
		}
		case ins_icmp://icmp语句
		{
			new_xcmp(option, line, num_func);
			break;
		}
		case ins_fcmp://fcmp语句
		{
			new_xcmp(option, line, num_func);
			break;
		}
		case ins_br://br语句
		{
			new_branch(option, line, num_func);
			break;
		}
		case ins_define:
		{
			break;
		}
		case ins_call://call语句
		{
			new_call(option, line, num_func);
			break;
		}
		case ins_ret://ret语句
		{
			new_ret(option, line, num_func);
			break;
		}
		case ins_label:
		{
			break;
		}
		case ins_unreachable://unreachable语句
		{
			new_unreachable(option, line, num_func);
			break;
		}
		case ins_sitofp://sitofp语句
		{
			new_xtoy(option, line, num_func);
			break;
		}
		case ins_fptosi://fptosi语句
		{
			new_xtoy(option, line, num_func);
			break;
		}
		case ins_zext://zext语句
		{
			new_zext(option, line, num_func);
			break;
		}
	}
}
