#include "riscv.h"

int num_start_func;//���������
Register_physical reg[2][num_registers];
map<type_label, int>map_global_register;//ȫ�ּĴ���������ŵ�ӳ��
map<int, Register_virtual*>map_global_register_position;//ȫ�ּĴ�����ŵ�ָ���ӳ��
map<int, Register_virtual*>map_local_register_position;//�ֲ�����Ĵ�����ŵ�ָ���ӳ��
map<int, std::string>map_physical_reg_name;//����Ĵ�����ŵ����Ƶ�ӳ��
vector<bool>physical_reg_usable[2];//����Ĵ����ܷ�ʹ��
vector<int>physical_reg_order[2];//���Ƿ�������Ĵ�����˳��
vector<int>physical_reg_saved[2];//special,caller_saved,callee_saved
int total_register = 0;

variable_table* global = new variable_table, * global_tail;
int total_global = 0;//�洢����������
map<type_variables, int>map_global;//����������ŵ�ӳ��
map<int, variable_table*>map_variable_position;//������ŵ�ָ���ӳ��
map<int, int>map_register_local;//�Ĵ�����ŵ�������ŵ�ӳ��
map<int, int>map_register_local_alloca;//alloca����ڼĴ�����ŵ�������ŵ�ӳ��

instruction* start = new instruction;//����ȫ�ֵ�ָ��
map<std::string, int>ins_num, cond_num;
int tot_instructions = 0;//�ܵ�ָ����
set<int>ins_definied;//�ᶨ������Ĵ�����ָ��(��call)
set<int>ins_used;
set<int>ins_valuate;//���Rd��ֵ��ָ��
set<int>asm_change_Rd;//��ı�Rd��ָ��
map<int, instruction*>map_instruction_position;//ָ���ŵ�ָ���ӳ��
map<int, pair<int, int> >map_ins_copy;//��¼copyָ���λ��
map<int, std::string>map_asm;//���ָ����뵽�ַ�ӳ��


map<type_label, int>label_num;
map<int, int>label_ins;//label��ָ���ӳ��
vector<basic_block*>label_bb;//label��Ŷ�Ӧ��bb���
int tot_label = 0;

functions* func_head = new functions, * func_tail = new functions;
map<type_label, int>map_function;
map<int, functions*>map_function_position;//���ݺ�����Ų��Һ���ָ��
int tot_functions = 0;//�ܵĺ�����

void init_definition()//��ʼ������
{
	ins_num = {
		{"global",0},{"load",1},{"store",2},{"alloca",3},{"getelementptr",4},
		{"add",5},{"fadd",6},{"sub",7},{"fsub",8},{"mul",9},{"fmul",10},{"sdiv",11},{"fdiv",12},
		{"srem",13},{"frem",14},{"and",15},{"or",16},{"xor",17}, {"fneg",18},{"icmp",19},
		{"fcmp",20},{"br",21},{"define",22},{"call",23},{"ret",24},{"label",25},
		{"unreachable",26},{"sitofp",27},{"fptosi",28},{"copy",29},{"sext",30}
	};
	cond_num = {
		{"eq",0},{"ne",1},{"ugt",2},{"sgt",3},{"ule",4},
		{"sle",5},{"uge",6},{"sge",7},{"ult",8},{"slt",9},
		{"oeq",10},{"ogt",11},{"oge",12}, {"olt",13},{"ole",14},{"une",15}
	};
	map_physical_reg_name = {
		{0,"zero"},{1,"ra"},{2,"sp"},{3,"gp"},{4,"tp"},{5,"t0"},{6,"t1"},{7,"t2"},
		{8,"s0"},{9,"s1"},{10,"a0"},{11,"a1"},{12,"a2"},{13,"a3"},{14,"a4"},{15,"a5"},
		{16,"a6"},{17,"a7"},{18,"s2"},{19,"s3"},{20,"s4"},{21,"s5"},{22,"s6"},{23,"s7"},
		{24,"s8"},{25,"s9"},{26,"s10"},{27,"s11"},{28,"t3"},{29,"t4"},{30,"t5"},{31,"t6"},
		{32,"ft0"},{33,"ft1"},{34,"ft2"},{35,"ft3"},{36,"ft4"},{37,"ft5"},{38,"ft6"},{39,"ft7"},
		{40,"fs0"},{41,"fs1"},{42,"fa0"},{43,"fa1"},{44,"fa2"},{45,"fa3"},{46,"fa4"},{47,"fa5"},
		{48,"fa6"},{49,"fa7"},{50,"fs2"},{51,"fs3"},{52,"fs4"},{53,"fs5"},{54,"fs6"},{55,"fs7"},
		{56,"fs8"},{57,"fs9"},{58,"fs10"},{59,"fs11"},{60,"ft8"},{61,"ft9"},{62,"ft10"},{63,"ft11"},
		{64,"fcsr"}
	};
	map_asm = {
		{0,"mv"},{1,"la"},{2,"li"},{3,"lw"},{4,"flw"},{5,"ld"},{6,"fld"}, {7,"sw"},{8,"fsw"},
		{9,"sd"},{10,"fsd"}, {11,"fmv.x.w"},{12,"fmv.w.x"},{13,"add"},{14,"addi"},{15,"sub"},
		{16,"subi"},{17,"mul"},{18,"div"},{19,"rem"}, {20,"and"},{21,"andi"},{22,"or"},{23,"ori"},
		{24,"xor"},{25,"xori"},{26,"sll"},{27,"srl"},{28,"sra"},{29,"slli"},{30,"srli"},{31,"srai"},
		{32,"addw"},{33,"addiw"},{34,"subw"},{35,"subiw"},{36,"mulw"},{37,"divw"},{38,"remw"},
		{39,"andw"},{40,"andiw"},{41,"orw"},{42,"oriw"},{43,"xorw"},{44,"xoriw"},{45,"sllw"},
		{46,"srlw"},{47,"sraw"},{48,"slliw"},{49,"srliw"},{50,"sraiw"},{51,"fadd.s"},{52,"fsub.s"},
		{53,"fmul.s"},{54,"fdiv.s"},{55,"frem.s"},{56,"fneg.s"},{57,"fcvt.s.w"},{58,"fcvt.w.s"},
		{59,"j"},{60,"jal"},{61,"jr"},{62,"beq"},{63,"bne"},{64,"blt"},{65,"bltu"} ,
		{66,"ble"},{67,"bleu"},{68,"bgt"},{69,"bgtu"},{70,"bge"},{71,"bgeu"},
		{72,"feq.s"},{73,"flt.s"},{74,"fle.s"},{75,"call"},{76,"fcmpe.s"},{77,"bgtz"},{78,"label"}
	};
	asm_change_Rd = {
		ins_asm_mv,ins_asm_la,ins_asm_li,ins_asm_lw,ins_asm_flw,ins_asm_ld,ins_asm_fld,
		ins_asm_fmv_x_w,ins_asm_fmv_w_x,ins_asm_add,ins_asm_addi,ins_asm_sub,ins_asm_subi,
		ins_asm_mul,ins_asm_div,ins_asm_rem,ins_asm_and,ins_asm_andi,ins_asm_or,ins_asm_ori,
		ins_asm_xor,ins_asm_xori,ins_asm_sll,ins_asm_srl,ins_asm_sra,ins_asm_slli,ins_asm_srli,
		ins_asm_srai,ins_asm_addw,ins_asm_addiw,ins_asm_subw,ins_asm_subiw,ins_asm_mulw,
		ins_asm_divw,ins_asm_remw,ins_asm_andw,ins_asm_andiw,ins_asm_orw,ins_asm_oriw,ins_asm_xorw,
		ins_asm_xoriw,ins_asm_sllw,ins_asm_srlw,ins_asm_sraw,ins_asm_slliw,ins_asm_srliw,
		ins_asm_sraiw,ins_asm_fadd_s,ins_asm_fsub_s,ins_asm_fmul_s,ins_asm_fdiv_s,ins_asm_frem_s,
		ins_asm_fneg_s,ins_asm_fcvt_s_w,ins_asm_fcvt_w_s,
		ins_asm_feq_s,ins_asm_flt_s,ins_asm_fle_s,ins_asm_fcmpe_s
	};
	/*physical_reg_order[0] = {
		t0,t1,t2
	};
	physical_reg_order[1] = {
		ft0,ft1,ft2
	};*/
	physical_reg_order[0] = {
		t0,t1,t2,t3,t4,t5,t6,s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,
		a7,a6,a5,a4,a3,a2,a1,a0,zero,ra,sp,gp,tp
	};
	physical_reg_order[1] = {
		ft0,ft1,ft2,ft3,ft4,ft5,ft6,ft7,ft8,ft9,ft10,ft11,
		fs0,fs1,fs2,fs3,fs4,fs5,fs6,fs7,fs8,fs9,fs10,fs11,
		fa7,fa6,fa5,fa4,fa3,fa2,fa1,fa0
	};
	physical_reg_usable[0] = {
		false,false,false,false,false,true,true,true,
		true,true,true,true,true,true,true,true,
		true,true,true,true,true,true,true,true,
		true,true,true,true,true,true,true,true
	};
	physical_reg_usable[1] = {
		true,true,true,true,true,true,true,true,
		true,true,true,true,true,true,true,true,
		true,true,true,true,true,true,true,true,
		true,true,true,true,true,true,true,true
	};
	physical_reg_saved[0] = {
		special,special,special,special,special,caller_saved,caller_saved,caller_saved,
		callee_saved,callee_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,
		caller_saved,caller_saved,callee_saved,callee_saved,callee_saved,callee_saved,callee_saved,callee_saved,
		callee_saved,callee_saved,callee_saved,callee_saved,caller_saved,caller_saved,caller_saved,caller_saved
	};
	physical_reg_saved[1] = {
		caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,
		callee_saved,callee_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,caller_saved,
		caller_saved,caller_saved,callee_saved,callee_saved,callee_saved,callee_saved,callee_saved,callee_saved,
		callee_saved,callee_saved,callee_saved,callee_saved,caller_saved,caller_saved,caller_saved,caller_saved
	};
	ins_definied = { 
		ins_load,ins_alloca,ins_getelementptr,ins_add,ins_fadd,ins_sub,ins_fsub,ins_mul,
		ins_fmul,ins_sdiv,ins_fdiv,ins_srem,ins_frem,ins_and,ins_or,ins_xor,ins_fneg,
		ins_icmp,ins_fcmp,ins_sitofp,ins_fptosi,ins_sext
	};
	ins_used = { ins_load,ins_getelementptr,ins_sitofp,ins_fptosi,ins_sext };
	ins_valuate = {
		ins_load,ins_alloca,ins_getelementptr,ins_add,ins_fadd,ins_sub,ins_fsub,ins_mul,
		ins_fmul,ins_sdiv,ins_fdiv,ins_srem,ins_frem,ins_and,ins_or,ins_xor,ins_fneg,
		ins_icmp,ins_fcmp,ins_sitofp,ins_fptosi,ins_sext
	};
	//��ʼ������ͷβָ��
	func_tail = func_head;
	//��ʼ��ȫ�ֱ���βָ��
	global_tail = global;
}

int main()
{
	std::string input_file;
	FILE* input_stream;
	FILE* output_stream;
	freopen_s(&input_stream, "file_name.in", "r", stdin);
	freopen_s(&output_stream, "asm.s", "w", stdout);
	init_definition();
	cin >> input_file;
	std::ifstream file(input_file);//���ļ�
	if (!file.is_open())
	{
		std::cerr << "�޷����ļ�" << std::endl;
		return 1;
	}
	std::string line;
	std::string s;
	functions* lst_func = func_head, * now_func = NULL;//ָ����һ��������ָ��ǰ����
	bool in_func = 0;//�Ƿ��ں����ڲ�����ʼ��Ϊ��
	std::string func_name;//������
	/*���ҳ�����ȫ�ֱ���*/
	while(getline(file,line))//���ж���
	{
		if (line[0] == ';')
			continue;
		std::istringstream word(line);
		while (word >> s)
		{
			if (check_label(s))//ȡ��label
			{
				type_label ret = func_name + get_label(s);
				tot_label++;
				label_num[ret] = tot_label;
			}
			int option = -1;//������û��ʶ���ָ����Ϊ-1
			if (ins_num.count(s) == 0)
				continue;
			else option = ins_num[s];
			if (option == ins_define)
				func_name = get_function_name(line);
			if (option == 0)
			{
				read(option, line, now_func, in_func);
				break;
			}
		}
	}
	label_bb.resize(tot_label + 1);
	for (int i = 0; i < label_bb.size(); ++i)
		label_bb[i] = NULL;
	std::ifstream file_2(input_file);
	while (getline(file_2, line))//���ж���
	{
		std::istringstream word(line);
		while (word >> s)
		{
			if (s[0] == ';')
				break;
			if (s[0] == '}')//���ﺯ�������ĩβ
			{
				get_basic_block(now_func);//�ҳ�basic blocks
				get_live_interval(now_func);//�ҳ�ÿ������Ĵ����Ļ�Ծ����
				register_allocate(now_func);//�Ĵ�������
				in_func = 0;
			}
			if (check_label(s))//��������label
			{
				type_label ret = now_func->name + get_label(s);
				new_label(ins_label, ret, now_func);
				line = get_new_line(line);
			}
			int option = -1;//������û��ʶ���ָ����Ϊ-1
			if (ins_num.count(s) == 0)
				continue;
			else option = ins_num[s];
			if (option == ins_define)//�����������������
			{
				functions* ret = new_function(line);
				if (ret->name == "@main")
					num_start_func = ret->num;
				map_function_position[ret->num] = ret;
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
	get_callee_saved_reg();
	now_func = func_head;
	while (now_func->next != NULL)
	{
		now_func = now_func->next;
		allocate_physical_reg(now_func);//Ϊ����Ĵ�������ջ�ռ�
		end_function(now_func);//���㺯������Ŀռ��С
	}
	get_asm();
	return 0;
}