@a = global i32 5

define i32 @add(i32 %a, i32 %b)
{
	%1 = alloca i32 23
	%2 = alloca i32 114514
	%3 = icmp eq i32 40,114514
	%3 = icmp ult i32 %1,233
	%3 = icmp sgt i32 %2,%1

	%4 = alloca float 1.234
	%5 = alloca float -22.452
	%6 = alloca float
	%6 = fcmp oeq float %4,%5
	%6 = fcmp ole float %4,-11.23
	%6 = fcmp une float 114.514,%5
	ret i32 %a
}