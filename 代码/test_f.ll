@b = global float -114.514
@a = global [5 x [3 x [2 x i32]]] 

; Function Attrs: noinline nounwind optnone uwtable
define i32 @add(i32 %0, i32 %1) {
  call i32 @add(i32 114514, i32 1919810)
  %3 = alloca i32
  %4 = alloca i32
  %5 = alloca i32
  store i32 %0, i32* %3
  store i32 %1, i32* %4
  %6 = load i32, i32* %3
  %7 = load i32, i32* %4
  %8 = add i32 %6, %7
  store i32 %8, i32* %5
  %9 = load i32, i32* %5
  %10 = load i32, i32* %4
  %11 = sub i32 %9, %10
  store i32 %11, i32* %5
  %12 = load i32, i32* %5
  %13 = load i32, i32* %4
  %14 = add i32 %12, %13
  store i32 %14, i32* %5
  %15 = load i32, i32* %5
  ret i32 %15 
}

; Function Attrs: noinline nounwind optnone uwtable
define float @mul(float %0, float %1) {
  %3 = alloca float
  %4 = alloca float
  %5 = alloca float
  store float %0, float* %3
  store float %1, float* %4
  %6 = load float, float* %3
  %7 = load float, float* %4
  %8 = mul float %6, %7
  store float %8, float* %5
  %9 = load float, float* %5
  ret float %9
}

; Function Attrs: noinline nounwind optnone uwtable
define void @fibonacci() {
  %1 = alloca i32
  %2 = alloca i32
  %3 = alloca i32
  %4 = alloca i32
  store i32 1, i32* %1
  store i32 1, i32* %2
  store i32 1, i32* %4
  br label %5

5:                                                ; preds = %14, %0
  %6 = load i32, i32* %4
  %7 = icmp sle i32 %6, 10
  br i1 %7, label %8, label %17

8:                                                ; preds = %5
  %9 = load i32, i32* %1
  store i32 %9, i32* %3
  %10 = load i32, i32* %1
  %11 = load i32, i32* %2
  %12 = add i32 %10, %11
  store i32 %12, i32* %1
  %13 = load i32, i32* %3
  store i32 %13, i32* %2
  br label %14

14:                                               ; preds = %8
  %15 = load i32, i32* %4
  %16 = add i32 %15, 1
  store i32 %16, i32* %4
  br label %5

17:                                               ; preds = %5
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() {
  %1 = alloca i32
  %2 = alloca i32
  %3 = alloca i32
  %4 = alloca [3 x [2 x float]]
  store i32 0, i32* %1
  store i32 -101, i32* %3
  %5 = load i32, i32* getelementptr ([5 x [3 x [2 x i32]]], [5 x [3 x [2 x i32]]]* @a, i32 0, i32 1, i32 2, i32 1)
  %6 = icmp eq i32 %5, 1
  br i1 %6, label %7, label %11

7:                                                ; preds = %0
  %8 = load i32, i32* getelementptr ([5 x [3 x [2 x i32]]], [5 x [3 x [2 x i32]]]* @a, i32 0, i32 1, i32 2, i32 0)
  %9 = load i32, i32* getelementptr ([5 x [3 x [2 x i32]]], [5 x [3 x [2 x i32]]]* @a, i32 0, i32 2, i32 1, i32 0)
  %10 = call i32 @add(i32 %8, i32 %9)
  store i32 %10, i32* %2
  br label %15

11:                                               ; preds = %0
  %12 = load i32, i32* getelementptr ([5 x [3 x [2 x i32]]], [5 x [3 x [2 x i32]]]* @a, i32 0, i32 0, i32 2, i32 0)
  %13 = load i32, i32* %3
  %14 = call i32 @add(i32 %12, i32 %13)
  store i32 %14, i32* %2
  br label %15

15:                                               ; preds = %11, %7
  call void @fibonacci()
  %16 = load i32, i32* %1
  ret i32 %16
}
