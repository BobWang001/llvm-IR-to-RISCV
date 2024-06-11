#include "riscv.h"

map<type_label, int>map_global_register;//全局寄存器名到编号的映射
int total_register = 0;

variable_table* global = new variable_table, * global_tail;
int total_global = 0;//存储全局变量的数量
map<type_variables, int>map_global;//变量名到编号的映射

instruction* start = new instruction;//属于全局的指令
map<std::string, int>ins_num, cond_num;
int tot_instructions = 0;//总的指令数

map<type_label, int>label_num;
int tot_label = 0;

functions* func_head = new functions, * func_tail = new functions;
map<type_label, int>map_function;
int tot_functions = 0;//总的函数数

void init_definition()//初始化定义
{
	ins_num = {
	{"global",0},{"load",1},{"store",2},{"alloca",3},{"getelementptr",4},
	{"add",5},{"sub",6},{"mul",7},{"sdiv",8},{"and",9},{"or",10},{"xor",11},
	{"icmp",12}, {"fcmp",13},{"br",14},{"define",15},{"call",16},{"ret",17},{"label",18}
	};
	cond_num = {
	{"eq",0},{"ne",1},{"ugt",2},{"sgt",3},{"ule",4},
	{"sle",5},{"uge",6},{"sge",7},{"ult",8},{"slt",9},
	{"oeq",10},{"ogt",11},{"oge",12}, {"olt",13},{"ole",14},{"une",15}
	};
	//初始化函数头尾指针
	func_tail = func_head;
	//初始化全局变量尾指针
	global_tail = global;
}

int main()
{
	init_definition();
	std::ifstream file("test_f.ll");//打开文件
	if (!file.is_open())
	{
		std::cerr << "无法打开文件" << std::endl;
		return 1;
	}
	std::string line;
	std::string s;
	functions* lst_func = func_head, * now_func = NULL;//指向上一个函数，指向当前函数
	bool in_func = 0;//是否在函数内部，初始化为否
	/*先找出所有全局变量*/
	while(getline(file,line))//逐行读入
	{
		if (line[0] == ';')
			continue;
		std::istringstream word(line);
		while (word >> s)
		{
			if (check_label(s))//取出label
			{
				type_label ret = get_label(s);
				tot_label++;
				label_num[ret] = tot_label;
			}
			int option = -1;//操作，没有识别出指令视为-1
			if (ins_num.count(s) == 0)
				continue;
			else option = ins_num[s];
			if (option == 0)
			{
				read(option, line, now_func, in_func);
				break;
			}
		}
	}
	std::ifstream file_2("test_f.ll");
	while (getline(file_2, line))//逐行读入
	{
		std::istringstream word(line);
		while (word >> s)
		{
			if (s[0] == ';')
				break;
			if (s[0] == '}')//到达函数定义的末尾
			{
				end_function(now_func);//计算函数所需的空间大小
				get_basic_block(now_func);//找出basic blocks
				in_func = 0;
			}
			if (check_label(s))//单独处理label
			{
				type_label ret = get_label(s);
				new_label(18, ret, now_func);
				line = get_new_line(line);
			}
			int option = -1;//操作，没有识别出指令视为-1
			if (ins_num.count(s) == 0)
				continue;
			else option = ins_num[s];
			if (option == 15)//单独处理函数定义语句
			{
				functions* ret = new_function(line);
				lst_func->next = ret;
				lst_func = ret;
				now_func = ret;
				in_func = 1;
				break;
			}
			else if (option != 0)
			{
				read(option, line, now_func, in_func);
				break;
			}
		}
	}
	return 0;
}
