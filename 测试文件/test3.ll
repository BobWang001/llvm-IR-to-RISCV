define i32 @gcd(i32 %0, i32 %1){
  %3 = alloca i32
  %4 = alloca i32
  %5 = alloca i32
  store i32 %0, i32* %4
  store i32 %1, i32* %5
  %6 = load i32, i32* %5
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %8, label %10

8:
  %9 = load i32, i32* %4
  store i32 %9, i32* %3
  br label %16

10:
  %11 = load i32, i32* %5
  %12 = load i32, i32* %4
  %13 = load i32, i32* %5
  %14 = srem i32 %12, %13
  %15 = call i32 @gcd(i32 %11, i32 %14)
  store i32 %15, i32* %3
  br label %16

16:
  %17 = load i32, i32* %3
  ret i32 %17
}

define i32 @mul(i32 %0, i32 %1) {
  %3 = alloca i32
  %4 = alloca i32
  %5 = alloca i32
  store i32 %0, i32* %3
  store i32 %1, i32* %4
  store i32 1, i32* %5
  br label %6

6:
  %7 = load i32, i32* %4
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %9, label %23

9:
  %10 = load i32, i32* %4
  %11 = and i32 %10, 1
  %12 = icmp ne i32 %11, 0
  br i1 %12, label %13, label %17

13:
  %14 = load i32, i32* %5
  %15 = load i32, i32* %3
  %16 = mul i32 %14, %15
  store i32 %16, i32* %5
  br label %17

17:
  %18 = load i32, i32* %3
  %19 = load i32, i32* %3
  %20 = mul i32 %18, %19
  store i32 %20, i32* %3
  %21 = load i32, i32* %4
  %22 = sdiv i32 %21, 2
  store i32 %22, i32* %4
  br label %6

23:
  %24 = load i32, i32* %5
  ret i32 %24
}

define i32 @main() {
  %1 = alloca i32
  %2 = alloca i32
  %3 = alloca i32
  %4 = alloca i32
  %5 = alloca i32
  %6 = alloca i32
  store i32 0, i32* %1
  store i32 4, i32* %2
  store i32 6, i32* %3
  store i32 1, i32* %4
  %7 = load i32, i32* %4
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %9, label %13

9:
  %10 = load i32, i32* %2
  %11 = load i32, i32* %3
  %12 = call i32 @mul(i32 %10, i32 %11)
  store i32 %12, i32* %5
  br label %17

13:
  %14 = load i32, i32* %3
  %15 = load i32, i32* %2
  %16 = call i32 @mul(i32 %14, i32 %15)
  store i32 %16, i32* %5
  br label %17

17:
  %18 = load i32, i32* %2
  %19 = load i32, i32* %3
  %20 = call i32 @gcd(i32 %18, i32 %19)
  store i32 %20, i32* %6
  %21 = load i32, i32* %5
  %22 = load i32, i32* %6
  %23 = add i32 %21, %22
  ret i32 %23
}
