define i32 @add(i32 %0, i32 %1) {
  %3 = alloca i32
  %4 = alloca i32
  store i32 %0, i32* %3
  store i32 %1, i32* %4
  %5 = load i32, i32* %3
  %6 = load i32, i32* %4
  %7 = add i32 %5, %6
  ret i32 %7
}

define i32 @main() {
  %1 = alloca i32
  %2 = alloca [100 x i32]
  %3 = alloca [100 x i32]
  %4 = alloca [100 x i32]
  %5 = alloca i32
  store i32 0, i32* %1
  store i32 0, i32* %5
  br label %6

6:
  %7 = load i32, i32* %5
  %8 = icmp slt i32 %7, 100
  br i1 %8, label %9, label %33

9:
  %10 = load i32, i32* %5
  %11 = load i32, i32* %5
  %12 = sext i32 %11 to i64
  %13 = getelementptr [100 x i32], [100 x i32]* %2, i64 0, i64 %12
  store i32 %10, i32* %13
  %14 = load i32, i32* %5
  %15 = load i32, i32* %5
  %16 = sext i32 %15 to i64
  %17 = getelementptr [100 x i32], [100 x i32]* %3, i64 0, i64 %16
  store i32 %14, i32* %17
  %18 = load i32, i32* %5
  %19 = sext i32 %18 to i64
  %20 = getelementptr [100 x i32], [100 x i32]* %2, i64 0, i64 %19
  %21 = load i32, i32* %20
  %22 = load i32, i32* %5
  %23 = sext i32 %22 to i64
  %24 = getelementptr [100 x i32], [100 x i32]* %3, i64 0, i64 %23
  %25 = load i32, i32* %24
  %26 = call i32 @add(i32 %21, i32 %25)
  %27 = load i32, i32* %5
  %28 = sext i32 %27 to i64
  %29 = getelementptr [100 x i32], [100 x i32]* %4, i64 0, i64 %28
  store i32 %26, i32* %29
  br label %30

30:
  %31 = load i32, i32* %5
  %32 = add i32 %31, 1
  store i32 %32, i32* %5
  br label %6

33:
  %34 = getelementptr [100 x i32], [100 x i32]* %4, i64 0, i64 99
  %35 = load i32, i32* %34
  ret i32 %35
}