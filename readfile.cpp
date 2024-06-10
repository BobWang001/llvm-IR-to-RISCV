#include "riscv.h"

extern int total_global;//存储全局变量的数量
extern variable_table* global, * global_tail;
extern int tot_instructions;//总的指令数
extern map<std::string, int>cond_num;

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
	num_func->ins_tail->next = new_instruction;
	num_func->ins_tail = new_instruction;
}

/*新申请一个局部变量*/
variable_table* get_new_local(functions* num_func, std::string word)
{
	variable_table* new_local = new variable_table;
	new_local->num = ++total_global;
	num_func->map_local[word] = total_global;
	new_local->name = word;
	new_local->cnt = 1;
	new_local->dim = 1;
	num_func->local_tail->next = new_local;
	num_func->local_tail = new_local;
	return new_local;
}

void new_variable_type(variable_table* new_global, std::string word)
{
	int len = word.length();
	for (int p = 0; p < len; )
	{
		if (word[p] == '[' || word[p] == ']' || word[p]=='x' || word[p] == ' ')
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
		if (word[p] == '[' || word[p] == ']' || word[p] == 'x' || word[p] == ' ')
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

void new_variable(int op, std::string line, variable_table* tail, functions* num_func = NULL)
{

	printf(";%s\n", line.c_str());

	variable_table* new_variable = new variable_table;
	new_variable->num = total_global+1;//记录编号
	bool is_ins = 0, name = 0, size_type = 0, val = 0;//标记是否已经找到了全局变量的名称，大小类型，初始值
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == 9)
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
				while (p < len && line[p] != ' ' && line[p] != ';')
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
			name = 1;
		}
		else if (!size_type)
		{
			if (word[0] != '[')
				new_variable->type = (word == "i32") ? 0 : 1;
			else
				new_variable_type(new_variable, word);
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
	if (op)
		num_func->map_local[new_variable->name] = new_variable->num;
	else map_global[new_variable->name] = new_variable->num;
	total_global += new_variable->cnt;
	//用0补齐
	while (new_variable->val.size() < new_variable->cnt)
		new_variable->val.push_back(0);
	new_variable->next = NULL;
	tail->next = new_variable;
	tail = new_variable;
	//加入局部变量
	if (op)
	{
		num_func->total_actual += new_variable->cnt;
		num_func->local_tail->next = new_variable;
		num_func->local_tail = new_variable;
	}

	printf(";name=%s,number=%d,type=%d,dim=%d,cnt=%d\n", new_variable->name.c_str(), new_variable->num, new_variable->type, new_variable->dim, new_variable->cnt);
	printf(";size: ");
	for (auto it : new_variable->size)
		printf("%u ", it);
	printf("\n;val: ");
	for (auto it : new_variable->val)
		printf("%u ", it);
	printf("\n\n");

}

void new_load(int op,std::string line,functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_load = new instruction;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs = 0, tRs = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "load")
			continue;
		if (!fRd)
		{
			if (map_global.count(word) != 0)
				new_load->Rd = map_global[word];
			else if (num_func->map_local.count(word) != 0)
				new_load->Rd = num_func->map_local[word];
			else//之前没有给该变量分配空间的话就分配一个局部变量
			{
				++num_func->total_actual;
				variable_table* new_local = get_new_local(num_func, word);
				new_load->Rd = num_func->map_local[word] ;
			}
			fRd = 1;
		}
		else if (!tRd)
		{
			new_load->tRd = ((word == "i32") ? 0 : 1);
			tRd = 1;
		}
		else if (!tRs)
		{
			new_load->tRs1 = ((word == "i32*") ? 0 : 1);
			tRs = 1;
		}
		else if (!fRs)
		{
			if (map_global.count(word) != 0)
				new_load->Rs1 = map_global[word];
			else
				new_load->Rs1 = num_func->map_local[word];
			fRs = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_load->num = ++tot_instructions;
	new_load->op = op;
	insert_instruction(num_func, new_load);

	printf(";Rd=%d type=%d Rs=%d type=%d\n", new_load->Rd, new_load->tRd, new_load->Rs1, new_load->tRs1);
	printf("\n");

}

void new_store(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_store = new instruction;
	bool is_ins = 0, fRd = 0, tRd = 0, fRs_imm = 0, tRs_imm = 0;//标记是否已经找到了目的变量及其type，源变量及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "store")
			continue;
		if (!tRs_imm)
		{
			new_store->tRs1 = ((word == "i32") ? 0 : 1);
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
				if (map_global.count(word) != 0)
					new_store->Rs1 = map_global[word];
				else
					new_store->Rs1 = num_func->map_local[word];
			}
			fRs_imm = 1;
		}
		else if (!tRd)
		{
			new_store->tRd = ((word == "i32*") ? 0 : 1);
			tRd = 1;
		}
		else if (!fRd)
		{
			if (map_global.count(word) != 0)
				new_store->Rd = map_global[word];
			else
				new_store->Rd = num_func->map_local[word];
			fRd = 1;
		}
	}
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_store->num = ++tot_instructions;
	new_store->op = op;
	insert_instruction(num_func, new_store);

	printf(";Rd=%d type=%d ", new_store->Rd, (new_store->tRd == 0) ? 0 : 1);
	if (new_store->fimm)
		printf("imm=%u ", new_store->imm);
	else printf("Rs=%d ", new_store->Rs1);
	printf("type=%d\n", (new_store->tRs1 == 0) ? 0 : 1);
	printf("\n");

}

void new_GEP(int op, std::string line, functions* num_func)
{

}

void new_operation(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_operation = new instruction;
	bool is_ins = 0, fRd = 0, fRs1_imm = 0, fRs2_imm = 0, type = 0;//标记是否已经找到了目的变量及其type，源变量(1/2)及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "add" || word == "sub" || word == "mul"
			|| word == "sdiv" || word == "and" || word == "or" || word == "xor")
			continue;
		if (!fRd)
		{
			if (map_global.count(word) != 0)
				new_operation->Rd = map_global[word];
			else if (num_func->map_local.count(word) != 0)
				new_operation->Rd = num_func->map_local[word];
			else//之前没有给该变量分配空间的话就分配一个局部变量
			{
				++num_func->total_actual;
				variable_table* new_local = get_new_local(num_func, word);
				new_operation->Rd = num_func->map_local[word];
			}
			fRd = 1;
		}
		else if (!type)
		{
			new_operation->tRd = new_operation->tRs1 = new_operation->tRs2 = ((word == "i32") ? 0 : 1);
			type = 1;
		}
		else if (!fRs1_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_operation->fimm1 = 1;
				if (new_operation->tRs1 == 0)
					new_operation->imm1 = atoi(word.c_str());
				else
					new_operation->imm1 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_operation->fimm1 = 0;
				if (map_global.count(word) != 0)
					new_operation->Rs1 = map_global[word];
				else
					new_operation->Rs1 = num_func->map_local[word];
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
				if (map_global.count(word) != 0)
					new_operation->Rs2 = map_global[word];
				else
					new_operation->Rs2 = num_func->map_local[word];
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
	printf("type=%d\n", (new_operation->tRd == 0) ? 0 : 1);
	printf("\n");
}

void new_xcmp(int op, std::string line, functions* num_func)
{

	printf(";%s\n", line.c_str());

	instruction* new_xcmp = new instruction;
	bool is_ins = 0, fRd = 0, cond = 0, fRs1_imm = 0, fRs2_imm = 0, type = 0;//标记是否已经找到了目的变量及其type，源变量(1/2)及其type
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "icmp" || word == "fcmp")
			continue;
		if (!fRd)
		{
			if (map_global.count(word) != 0)
				new_xcmp->Rd = map_global[word];
			else if (num_func->map_local.count(word) != 0)
				new_xcmp->Rd = num_func->map_local[word];
			else//之前没有给该变量分配空间的话就分配一个局部变量
			{
				++num_func->total_actual;
				variable_table* new_local = get_new_local(num_func, word);
				new_xcmp->Rd = num_func->map_local[word];
			}
			fRd = 1;
		}
		else if (!cond)
		{
			new_xcmp->cond = cond_num[word];
			cond = 1;
		}
		else if (!type)
		{
			new_xcmp->tRd = new_xcmp->tRs1 = new_xcmp->tRs2 = ((word == "i32") ? 0 : 1);
			type = 1;
		}
		else if (!fRs1_imm)
		{
			if (word[0] >= '0' && word[0] <= '9' || word[0] == '-')//存在立即数
			{
				new_xcmp->fimm1 = 1;
				if (new_xcmp->tRs1 == 0)
					new_xcmp->imm1 = atoi(word.c_str());
				else
					new_xcmp->imm1 = floatToBinary(atof(word.c_str()));
			}
			else//非立即数
			{
				new_xcmp->fimm1 = 0;
				if (map_global.count(word) != 0)
					new_xcmp->Rs1 = map_global[word];
				else
					new_xcmp->Rs1 = num_func->map_local[word];
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
				if (map_global.count(word) != 0)
					new_xcmp->Rs2 = map_global[word];
				else
					new_xcmp->Rs2 = num_func->map_local[word];
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
	printf("cond=%d type=%d\n", new_xcmp->cond, (new_xcmp->tRd == 0) ? 0 : 1);
	printf("\n");
}

void new_branch(int op, std::string line, functions* num_func)
{

}

/*获取call指令的参数列表*/
void get_instruction_args(instruction* ins, std::string line, functions* num_func)
{
	int len = line.length();
	bool last_type = 0;//当前参数类型
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ',')
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
			if (map_global.count(word) != 0)
				ins->formal_num.push_back(map_global[word]);
			else ins->formal_num.push_back(num_func->map_local[word]);
		}
	}
}

void new_call(int op, std::string line, functions* num_func)
{
	
	printf(";%s\n", line.c_str());

	instruction* new_call = new instruction;
	bool is_ins = 0, fRd = 0, tRd = 0, name = 0;//标记是否已经找到了目的变量及其type，被调函数名
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';' || line[p] == '(')
			break;
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格或者'('停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != '(')
		{
			word.push_back(line[p]);
			p++;
		}
		if (word == "=" || word == "call")
			continue;
		if (!fRd)
		{
			if (word == "void")
			{
				new_call->type_ret = 2;
				tRd = 1;
			}
			else
			{
				if (map_global.count(word) != 0)
					new_call->Rd = map_global[word];
				else if (num_func->map_local.count(word) != 0)
					new_call->Rd = num_func->map_local[word];
				else//之前没有给该变量分配空间的话就分配一个局部变量
				{
					++num_func->total_actual;
					variable_table* new_local = get_new_local(num_func, word);
					new_call->Rd = num_func->map_local[word];
				}
			}
			fRd = 1;
		}
		else if (!tRd)
		{
			new_call->type_ret = ((word == "i32") ? 0 : 1);
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
	if (!is_ins)
		return;
	num_func->cnt_ins++;
	new_call->num = ++tot_instructions;
	new_call->op = op;
	insert_instruction(num_func, new_call);

	printf(";type=%d arg_cnt=%d ", new_call->type_ret, new_call->tot_formal);
	if (new_call->type_ret != 2)
		printf("ret_num=%d\n", new_call->Rd);
	else printf("\n");
	int nw = 0;
	for (auto it : new_call->formal_num)
	{
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
		if (line[p] == ' ' || line[p] == ',' || line[p] == 9)
		{
			p++;
			continue;
		}
		if (line[p] == ';')
			break;
		/*获得单词：其他均为读到空格停止*/
		is_ins = 1;
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ';' && line[p] != ',')
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
			if (map_global.count(word) != 0)
				new_ret->Rs1 = map_global[word];
			else new_ret->Rs1 = num_func->map_local[word];
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
		if (line[p] == ' ' || line[p] == ',')
		{
			p++;
			continue;
		}
		std::string word;
		while (p < len && line[p] != ' ' && line[p] != ',')
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
		else//参数名,新增一个局部变量
		{
			variable_table* new_local = get_new_local(new_function, word);
			new_local->type = last_type;
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
		if (line[p] == ' ' || line[p] == ',')
		{
			p++;
			continue;
		}
		if (line[p] == ';' || line[p] == '(')
			break;
		/*获得单词：除了存在小括号进行括号匹配以外，其他均为读到空格停止*/
		std::string word;
		while (p<len && line[p] != ' ' && line[p] != ';' && line[p] != ',' && line[p] != '(')
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
	//计算栈帧大小,多留了4字节
	now_function->size = (4 + now_function->tot_arg + 
		max(now_function->max_formal - 8, 0) + 1) << 2;
	//为局部变量分配空间,只写偏移量
	int imm = 20;
	variable_table* head = now_function->local_head;
	while (head->next != NULL)
	{
		head = head->next;
		now_function->imm_local[head->num] = -imm;
		imm += (head->cnt << 2);
	}

	printf(";size=%d\n", now_function->size);
	head = now_function->local_head;
	while (head->next != NULL)
	{
		head = head->next;
		printf(";name=%s imm=%d\n", head->name.c_str(), now_function->imm_local[head->num]);
	}
	printf("\n");

}

void read(int option, std::string line, functions* num_func, bool in_func)//读入
{
	switch (option)
	{
		case 0://global语句
		{
			new_variable(0, line, global_tail);//新建一个全局变量
			break;
		}
		case 1://load语句
		{
			new_load( 1, line, num_func);
			break;
		}
		case 2://store语句
		{
			new_store( 2, line, num_func);
			break;
		}
		case 3://alloca语句
		{
			new_variable(1, line, num_func->local_tail, num_func);
			break;
		}
		case 4://GEP语句
		{
			new_GEP( 4, line, num_func);
			break;
		}
		case 5://add语句
		{
			new_operation(5, line, num_func);
			break;
		}
		case 6://sub语句
		{
			new_operation(6, line, num_func);
			break;
		}
		case 7://mul语句
		{
			new_operation(7, line, num_func);
			break;
		}
		case 8://sdiv语句
		{
			new_operation(8, line, num_func);
			break;
		}
		case 9://and语句
		{
			new_operation(9, line, num_func);
			break;
		}
		case 10://or语句
		{
			new_operation(10, line, num_func);
			break;
		}
		case 11://xor语句
		{
			new_operation(11, line, num_func);
			break;
		}
		case 12://icmp语句
		{
			new_xcmp(12, line, num_func);
			break;
		}
		case 13://fcmp语句
		{
			new_xcmp(13, line, num_func);
			break;
		}
		case 14://br语句
		{
			new_branch( 14, line, num_func);
			break;
		}
		case 16://call语句
		{
			new_call( 16, line, num_func);
			break;
		}
		case 17://ret语句
		{
			new_ret( 17, line, num_func);
			break;
		}
	}
}
