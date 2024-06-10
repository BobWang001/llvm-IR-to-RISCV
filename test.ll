@a = global i32 5
;@a = global i32 5
@b = global float -5.32;get a new variable
@c = global [2 x [3 x i32]] [[3 x i32][i32 11,i32 -221,i32 312],[3 x i32][i32 -1,i32 -20,i32 35]]
@d = global [3 x [2 x float]] [[2 x float][float 1.0,float 2.21],[2 x float][float -3.21,float 4],[2 x float][float 10,float -123.456]]
@e = global [8 x i32];hello from the other side

define i32 @add(i32 %a, i32 %b)
{
	%1 = alloca i32
	%2 = alloca i32
	%3 = add i32 40,114514
	%3 = xor i32 1,%3
	%4 = alloca float 1.234
	%5 = alloca float -22.452
	%6 = alloca float
	%6 = sdiv float %4,%5
	%6 = mul float %5,%6
	%6 = sub float %6,%6
	%6 = sdiv float %6,191.9810
	ret i32 %a
}

define float @max(float %a, float %b,float %c)
{
	%1 = alloca i32
	%2 = alloca i32
	%1 = load i32,i32* @a
	store i32 %1,i32* %2
	store i32 10,i32* %1
}

define i32 @fibonacci(i32 %a, i32 %b, i32 %c )
{
	%c = call i32 @add(i32 %a ,i32 %b)
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
}

define void @imsb(i32 %a,float %b)
{
	%1 = alloca [3 x [2 x float]]
	%b = i32 load i32* %1
	ret void
}