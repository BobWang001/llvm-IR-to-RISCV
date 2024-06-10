#include "riscv.h"

extern int total_global;//存储全局变量的数量
extern variable_table* global, * global_tail;

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

void new_global_type(variable_table* new_global, std::string word)
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

void new_global_value(variable_table* new_global, std::string word)
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

void new_global(std::string line)
{

	printf(";%s\n", line.c_str());

	variable_table* new_global = new variable_table;
	new_global->num = ++total_global;//记录编号
	bool name = 0, size_type = 0, val = 0;//标记是否已经找到了全局变量的名称，大小类型，初始值
	int cnt = 0;
	int len = line.length();
	for (int p = 0; p < len; )
	{
		if (line[p] == ' ')
		{
			p++;
			continue;
		}
		/*获得单词：除了存在中括号进行括号匹配以外，其他均为读到空格停止*/
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
				break;
			}
			default:
			{
				p++;
				while (p < len && line[p] != ' ')
				{
					word.push_back(line[p]);
					p++;
				}
				break;
			}
		}
		p++;
		if (word == "global")
			continue;
		if (word == "=")
			continue;
		if (!name)
		{
			new_global->name = word;
			name = 1;
		}
		else if (!size_type)
		{
			if (word[0] != '[')
				new_global->type = (word == "i32") ? 0 : 1;
			else
				new_global_type(new_global, word);
			size_type = 1;
		}
		else if (!val)
		{
			if (word[0] != '[')
			{
				if (new_global->type == 0)//整数直接赋值
					new_global->val.push_back(atoi(word.c_str()));
				else//浮点数需要转换一下
					new_global->val.push_back(floatToBinary(atof(word.c_str())));
			}
			else
				new_global_value(new_global, word);
			val = 1;
		}
	}
	new_global->next = NULL;
	global_tail->next = new_global;
	global_tail = new_global;

	printf(";name=%s,type=%d,dim=%d,cnt=%d\n", new_global->name.c_str(), new_global->type, new_global->dim, new_global->cnt);
	printf(";");
	for (auto it : new_global->size)
		printf("%u ", it);
	printf("\n;");
	for (auto it : new_global->val)
		printf("%u ", it);
	printf("\n");

}

void read(int option, std::string line, functions* num, bool in_func)//读入
{
	switch (option)
	{
		case 0://global语句
		{
			new_global(line);//新建一个全局变量
			break;
		}
	}
}
