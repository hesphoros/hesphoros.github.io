段（sections）与常用伪指令（GAS / GNU as 风格）

- .text
  - 存放可执行代码。通常在函数前用 .global / .globl 导出符号，如 .global main 或 .global _start。
  - 常配 .align 以保证指令对齐（见下）。
- .data
  - 已初始化的可读写数据（全局变量、静态变量有初始值）。
- .bss
  - 未初始化或初始为 0 的静态/全局变量（占地址空间但不占可执行文件中初始化数据的大小）。
  - 常用 .space / .skip / .zero 为 bss 分配空间：例如 .space 64。
- 字符串与只读数据
  - .ascii "text" — 不以 NUL 结尾。
  - .asciz / .string "text" — 自动以 '\0' 结尾（常用于 C 字符串）。
  - .rodata — 只读常量段（有些链接器脚本将 .rodata 放在只读段）。
- 对齐
  - .align n — 在 GNU as（ARM）中，一般把 n 视为 2 的指数；例如 .align 2 会对齐到 4 字节边界（不同汇编器差异请注意）。
- 数据定义
  - .word <expr> — 定义 32-bit 常量或地址字面值。
  - .hword / .short — 16-bit
  - .byte — 8-bit
  - .float / .double — 浮点字面量
  - .space / .skip / .zero — 分配未初始化或置 0 空间
- 符号可见性与元信息
  - .global / .globl name — 导出符号
  - .local — 局部符号（默认）
  - .type symbol, %function / %object，.size symbol, .-symbol — 用于调试符号信息（ELF）

2) 寄存器（ARM32）
- 通用寄存器： r0, r1, r2, r3 — 用作函数参数和返回值（r0 返回，r0–r3 参数）。
- r4–r11 — callee-saved（被调用者需要保存/恢复）。
- r12 — IP（intra-proc-call scratch）。
- r13 — SP（栈指针）。
- r14 — LR（链接寄存器，保存返回地址）。
- r15 — PC（程序计数器）。
- CPSR — 程序状态寄存器（条件标志 NZCV 等）。

3) 调用约定（AAPCS / ARM EABI 要点）
- 参数：前 4 个参数放 r0–r3，额外参数通过栈传递。
- 返回值：r0（64-bit 返回需要 r0:r1）。
- 调用者保存（caller-saved）：r0–r3, r12
- 被调用者保存（callee-saved）：r4–r11, sp, lr（函数若使用这些寄存器须保存/恢复）
- 函数入口必须保证栈在公共接口处是 8 字节对齐（即在调用外部函数前 SP % 8 == 0）。
- 返回：通常用 BX LR 或 POP {.., pc}。

4) 常用指令分类与详细说明（含语法、标志）
- 数据处理指令（大多影响条件标志可选）
  - MOV{cond}{S} Rd, Operand2 — 把操作数写入 Rd。例： MOV r0, r1；MOVS r0, #0 会更新标志。
  - MVN — 取反（bitwise NOT）
  - AND / ORR / EOR / BIC — 逻辑运算
  - ADD / ADC / SUB / SBC / RSB — 算术（ADC/SBC 带进位）
  - CMP / CMN / TST / TEQ — 比较/测试指令，仅影响标志（无结果写回）
  - MUL / MLA — 乘法（MLA 带累加）
  - 注意：大多数指令可以带条件后缀（见条件码），也可带 S 后缀决定是否更新标志。
- 载入/存储
  - LDR Rt, [Rn] ; LDR Rt, [Rn, #imm] ; LDR Rt, [Rn, Rm] — 32-bit
  - STR — 存储 32-bit
  - LDRB / STRB — 字节
  - LDRH / STRH — 半字
  - LDRD / STRD — 双字（在某些变体）
  - LDM / STM — 多寄存器载入/存储（例：STMFD SP!, {r4-r7, lr}）
  - 预/后索引： LDR r0, [r1, #8]! （写回）； LDR r0, [r1], #4 （后索引写回）
  - PC 相对字面池： LDR r0, =const — 汇编器把它翻译为 PC 相对 LDR（字面池），或 ADR/ADD 视距离与对齐
  - PC 的值在 ARM 状态下用于数据访问时通常是当前指令地址 + 8（取决于流水线），这点要注意（直接使用 PC 作为基址时要补偿偏移）
- 分支与子程序
  - B label — 无条件跳转
  - B<cond> label — 条件跳转（EQ, NE, LT, LE, GT, GE, CS/HS, CC/LO, MI, PL, VS, VC, HI, LS）
  - BL label — 分支并链接（调用子函数，LR <- next instruction）
  - BX reg — 跳转到寄存器，低位 LSB 决定跳入 Thumb（若 LSB==1）或 ARM（LSB==0），用来在 Thumb/ARM 之间切换
- 条件执行
  - ARM（非 Thumb）下多数指令都可以带条件码后缀（例如 ADDEQ, MOVEQ 等）以减少分支开销。条件码基于 CPSR 的 NZCV。
- 特殊/系统
  - SVC / SWI — 软中断，发起系统调用（常见语法：svc #0）
  - CPS / MRS / MSR — 处理 CPSR 等（用于更底层控制）
- 立即数限制（重要）
  - ARM 数据处理指令的 12-bit 立即值编码为 8-bit 立即数 + 4-bit 旋转量（把 8-bit 立即数按偶数位右旋 0..30 位）。因此并非所有 32-bit 常数都能直接编码。
  - 常见对策：
    - 使用 MOVW / MOVT（assembler pseudo-instructions on ARM GNU toolchain）分别写入低 16 / 高 16 位，组合成 32 位立即数。
    - 使用字面池（LDR Rd, =const）让汇编器把常数放在代码附近并用 PC 相对 LDR 载入。
    - 组合移位与逻辑指令构造常数（ORR/MOV/MVN/MOVK 等在不同工具链中）。
- PC 值与字面池
  - 当使用 LDR Rd, [PC, #imm] 或汇编伪指令 LDR Rd, =label，实际访问的是 PC（当前指令地址 + 8）作为基址，加上偏移，因此要留意距离与对齐（汇编器会在代码中插入字面池）。
- 多寄存器载入/存储 (STM/LDM)
  - 用于保存/恢复寄存器组，或函数入栈出栈。例如： STMFD SP!, {r4-r7, lr} 将 r4–r7 与 lr 推到栈上； LDMFD SP!, {r4-r7, pc} 恢复并返回。

5) 条件码（Z N C V）与常用后缀
- EQ — equal (Z==1)
- NE — not equal (Z==0)
- CS/HS — carry set / unsigned higher or same (C==1)
- CC/LO — carry clear / unsigned lower (C==0)
- MI / PL — negative / positive or zero (N==1 / N==0)
- VS / VC — overflow set / clear (V==1 / V==0)
- HI / LS — unsigned higher / lower or same
- GE / LT — signed greater or equal / less than (N == V)
- GT / LE — signed greater than / less or equal
- AL — always（默认）

6) 栈帧与常见函数模板（约定式）
- 典型简单函数（无本地变量，仅保存 LR）：
    push {lr}
    ... body ...
    pop {pc}
  （pop {pc} 跳回，因为 pc 被弹回）
- 带本地变量的标准帧（AAPCS 推荐）：
    push {fp, lr}
    add fp, sp, #4
    sub sp, sp, #<locals_size>   @ 给局部变量分配空间
    ... 使用 fp 相对寻址 ...
    mov sp, fp
    pop {fp, pc}
- 使用 LDM/STM 也常见： STMFD SP!, {r4-r7, lr} / LDMFD SP!, {r4-r7, pc}

7) 常见“坑”和调试提示
- 立即数编码限制：试图写入某些常数会被 assembler 折成字面池或组合指令—要知道 assembler 的伪指令（=const）会生成字面池。
- PC 相对寻址偏移：PC 在 ARM 指令中被视为当前指令地址 + 8（pipeline），因此 LDR Rd, [PC, #offset] 计算地址时需考虑 +8。
- 对齐问题：访问未对齐的内存可能导致性能下降或异常（取决于处理器与访问类型）。确保数据与 4 字节对齐（结构、函数表等）。
- 栈对齐：在外部可见函数调用前 SP 必须 8 字节对齐，否则调用 C 库函数会导致问题（浮点、NEON 调用约定依赖于对齐）。
- ARM/Thumb 切换：使用 BX lr 返回时 lr 的最低位如果为 1 表示切换到 Thumb，若要确保在 ARM 状态返回，确保 lr 低位为 0。
- 使用条件执行来减少分支，但不要滥用以免影响可读性与流水线。

8) 示例：完整的 ARM32 汇编程序（Linux 下使用 sys_write / svc 0）
下面给出一个可直接汇编并链接的示例（GNU as + ld）。我把文件用代码块包含，文件名为 arm32_example.s。

```asm name=arm32_example.s
    .section .text
    .global _start
    .align 2

_start:
    /* write(STDOUT_FILENO=1, msg, len) syscall */
    mov     r0, #1              /* fd = 1 (stdout) */
    ldr     r1, =msg            /* buf = &msg  (assembler may emit PC-relative LDR) */
    ldr     r2, =msg_end        /* temp: address of msg_end */
    sub     r2, r2, r1          /* r2 = msg_end - msg => length */
    mov     r7, #4              /* syscall: sys_write (ARM Linux) */
    svc     #0

    /* exit(0) */
    mov     r0, #0
    mov     r7, #1              /* syscall: sys_exit */
    svc     #0

    .section .data
    .align 2
msg:
    .asciz  "Hello, ARM32 world!\n"
msg_end:

    .section .bss
    .align 2
buffer:
    .space 64
```

- 说明：
  - .asciz 会自动添加 NUL。msg_end 是一个空标签，用于计算字符串长度（msg_end - msg）。
  - ldr r1, =msg 是汇编伪指令，会被转成合适的 ADR/LDR literal（字面池）或 MOVW/MOVT，取决于常量距离与 assembler。
  - mov r7, #4 / svc #0 是传统在 ARM Linux 下发起系统调用的方式（r7 保存 syscall 编号，r0..r6 参数）。常见 sys_write 编号是 4，sys_exit 是 1（不同系统/ABI 请确认）。
  - 若链接到 libc 使用 main 而不是 _start，需要改写入口与调用（例如把 .global main 并返回到 libc）。

9) 逐条指令示例与解释（摘取常用）
- MOV r0, #1 — 把立即数 1 放入 r0（若立即数不被编码允许，汇编器会使用其他方式）。
- LDR r1, =msg — 用于载入地址常量（伪指令）。
- SUB r2, r2, r1 — r2 = r2 - r1，更新标志（若无 S 则不更新）。
- MOV r7, #4 / SVC #0 — 系统调用约定（ARM Linux 下）。
- PUSH/POP 是伪指令，等同于 STM/ LDM： PUSH {r4, r5, lr} -> STMFD SP!, {r4, r5, lr}。

-- PUSH/POP（ARM 伪指令）详解

- 语义与等价指令：
  - `push {reglist}` 是 GNU as 的伪指令，等价于 `stmfd sp!, {reglist}`（Full Descending 堆栈生长方向）。
  - `pop {reglist}` 等价于 `ldmfd sp!, {reglist}`（从栈中弹出并恢复寄存器）。

- 栈方向与约定：
  - 在 ARM 体系中，堆栈通常是向低地址生长（SP 减小时分配内存）。`stmfd sp!, {...}` 将先把 SP 减去相应字节数，然后把寄存器存入新栈顶。
  - `stmfd` 中的 `fd` 表示 Full (先写入整个寄存器组) 和 Descending（向低地址生长）。

- 寄存器存储顺序：
  - 当你执行 `push {r4, r5, lr}`，汇编器会按照寄存器编码的顺序写入内存（通常是从最低编号到最高编号），但**不要依赖于存储顺序**实现逻辑；只需依赖 pop 时以相同 reglist 恢复即可。

- LR / PC 的特殊用法：
  - `push {lr}` 常用于保存返回地址；用 `pop {pc}` 可以把弹出的值直接写入 PC，从而做返回（等同于 `bx lr` 效果，但会切换状态取决于最低位）。
  - 小心在 Thumb/ARM 状态切换时的最低位问题（PC/ LR 低位表示 Thumb 状态）。

- 示例：标准函数栈帧

```asm
push {fp, lr}       @ 保存帧指针与返回地址
add fp, sp, #4      @ 设置帧指针（fp 指向旧 fp 存放处）
sub sp, sp, #16     @ 分配局部变量
...                 @ 函数体
mov sp, fp
pop {fp, pc}        @ 恢复帧指针并返回
```

- 对齐与 ABI 要求：
  - 在调用外部函数（对 C 库等）之前，SP 必须满足 8 字节对齐（AAPCS 要求）。使用 push/pop 保持对齐很常见，但在分配局部空间时要确保最终对齐仍满足 ABI。

- 常见坑与调试提示：
  - 不要在 reglist 中重复寄存器（汇编器通常会报错或忽略重复项）。
  - 在同时使用 pop 恢复到 pc 的情况下，保证栈中的值确实为合法的返回地址（否则会跳转到错误地址）。
  - 当使用 `push {r0-r3, r12, lr}` 等大列表时，请注意栈增长与栈溢出风险（尤其在嵌入式系统上）。


## 进阶主题（简要）

- Position Independent Code (PIC) / GOT / PLT：在生成共享库或 PIE 时，访问全局变量/函数通常通过 GOT/PLT，需要额外的寄存器约定（如 r9 用作 GOT 基址在部分 ABI）。
- Thumb 与 ARM 混合：当从 ARM 跳入 Thumb（或反之）时，注意地址最低位含状态位（LSB=1 -> Thumb）。BX 指令会根据目标寄存器低位切换状态。
- NEON / VFP 指令：浮点与 SIMD 有自己的约定与寄存器（需要启用浮点 ABI）。
- 编译器伪指令：MOVW/MOVT （用于 32-bit 立即数构造），ARM GNU as 支持这些伪指令。


🧠 二、C 调用约定（cdecl） — 32 位 x86

下面是针对 32-bit x86 平台下常见的 cdecl（C 默认调用约定）的要点说明与示例，方便在汇编与 C 混合编程时参考。

- 基本原则：
  - 参数从右向左压入栈（caller pushes arguments right-to-left），即先 push 最右边的参数，最后 push 最左边的参数；
  - 调用者（caller）负责清理栈（通过 add esp, N），这允许可变参数函数（如 printf）运行；
  - 函数返回值放在 `EAX`（整型/指针）；对于 64-bit 整数或长长类型，一般使用 `EDX:EAX`（高 32 位放 EDX，低 32 位放 EAX）；复合类型（结构体/数组）通常由编译器通过隐藏指针返回（caller 分配返回空间并把地址作为隐含第一个参数传入），但具体行为依编译器/ABI 而异。

- 寄存器约定：
  - Caller-saved（调用者须保存，函数可能被覆盖）：EAX, ECX, EDX。
  - Callee-saved（被调用者需在返回前恢复）：EBX, ESI, EDI, EBP（ESP 始终必须被恢复到调用前的值）。
  - ESP 是栈指针，调用 `call` 指令会把返回地址压入栈（EIP），`ret` 会弹出返回地址并跳转。

- 栈帧与常见模版（callee）示例：

```asm
; callee (standard prologue/epilogue)
func:
    push ebp           ; 保存旧帧指针
    mov  ebp, esp      ; 建立新的帧指针
    sub  esp, <locals> ; 分配局部变量
    ; ... 函数体 ...
    mov  esp, ebp
    pop  ebp
    ret                ; 返回，ret 会弹出返回地址到 EIP
```

或者可用 `leave` 指令简写：

```asm
    leave
    ret
```

- 调用者（caller）示例：

```asm
    push dword arg3
    push dword arg2
    push dword arg1
    call  func
    add   esp, 12  ; caller cleans the stack (3 args * 4 bytes)
    ; 函数返回值在 EAX
```

- 可变参数函数（variadic）与类型提升：
  - 对于可变参数（如 printf），被调函数不知道参数数量，故调用者必须负责清理栈（cdecl 的一个原因）。
  - 在 cdecl 下，`float` 会被提升为 `double` 后压入栈；`char`/`short` 等整数类型会按 `int` 扩展后压栈（默认整型提升）。

- 结构体/聚合返回：
  - 大多数编译器对于较大的聚合类型（结构体、数组）使用隐藏指针由 caller 分配返回空间并把地址作为第一个参数传入 callee；callee 在该地址写入结果并在 EAX 中返回该地址（或仅返回）——具体规则因 ABI/编译器而异。
  - 对于小的 POD（例如最多 8 字节的简单聚合），某些编译器可能使用寄存器对（EDX:EAX）返回，但不要依赖这一行为，除非文档明确说明。

- 名称修饰与平台差异：
  - 在 Windows 32-bit（MSVC）下，符号通常以前导下划线 `_` 开头（例如 C 函数 `foo` 在目标文件中可能为 `_foo`）；而在 Linux (gcc) 下通常无前导下划线（或由 linker/objfmt 决定）。
  - Windows 下还有 `__stdcall`（由 callee 清理栈）与 `__cdecl` 的差别：stdcall 符号常带尾缀 `@N` 表示参数字节数（例如 `@12`），而 cdecl 由 caller 清理栈。

- 对齐要求：
  - 传统 x86 cdecl 要求 4 字节对齐（32-bit word alignment）；
  - 但现代编译器/优化（尤其使用 SSE/stack realignment）可能会在函数入口保证更严格的对齐（例如 16 字节）以满足 SIMD 指令要求；在编写必须与编译器 ABI 互操作的汇编时请参考目标编译器文档并保持相同对齐策略。

- 与汇编互操作的实用提示：
  - 在汇编中调用 C 函数前，确保把参数按正确顺序压栈并在 call 后清理栈（或使用 callee-cleaning 调用约定）。
  - 被 C 代码调用的汇编函数应在进入时保存 callee-saved 寄存器并在返回前恢复它们。
  - 如果函数会被 C++ 调用或与 C++ 链接，使用 `extern "C"` 来禁止 C++ 的名符修饰（name mangling）。

- 调试与验证：
  - 在 GDB 中可以使用 `disas`、`info registers`、`x/16x $esp` 来查看栈和寄存器并验证调用约定是否正确。
  - 用一个简单的 C 测试函数（在 C 中打印参数并返回结果）来验证汇编调用是否按预期传参与返回。

示例：汇编定义一个被 C 调用的简单函数（Linux/gcc 风格）：

```asm
    .global my_add
my_add:
    push ebp
    mov  ebp, esp
    mov  eax, [ebp+8]   ; first arg (arg1)
    add  eax, [ebp+12]  ; second arg (arg2)
    pop  ebp
    ret
```

在 C 中调用：

```c
extern int my_add(int a, int b);
int x = my_add(2, 3); // x == 5
```

以上内容覆盖了在 32 位 x86 下使用 cdecl 时大多数需要注意的要点。对于更严格的 ABI 细节（例如特定平台返回小聚合的规则、GCC 的 `regparm` 扩展等），请查阅目标平台的 ABI 文档与编译器手册。

