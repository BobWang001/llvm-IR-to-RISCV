运行程序：

`./llvm_to_riscv.exe`

输入`*.ll`

`riscv64-unknown-elf-as -o asm.o asm.s`

`riscv64-unknown-elf-ld -nostdlib -nostartfiles -o test.exe asm.o`

`./rvlinux test.exe`



在命令行内输入`./make.exe`，

再输入`*.ll`，

最后输入`./rvlinux test.exe`即可运行程序。
