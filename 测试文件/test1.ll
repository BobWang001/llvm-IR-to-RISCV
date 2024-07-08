define i32 @main() {
  %1 = alloca i32
  %2 = alloca i32
  %3 = alloca i32
  %4 = alloca i32
  store i32 0, i32* %1
  store i32 114, i32* %2
  store i32 54, i32* %3
  %5 = load i32, i32* %2
  %6 = load i32, i32* %3
  %7 = add i32 %5, %6
  store i32 %7, i32* %4
  %8 = load i32, i32* %4
  ret i32 %8
}