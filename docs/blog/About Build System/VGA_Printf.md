# 在 VGA 文本模式下实现最基础的 printf（物理地址 0xB8000）

本文说明如何在 x86 实模式与保护模式下使用 VGA 文本缓冲区（physical 0xB8000）来实现最基础的 printf 功能，包含内存布局、字符与属性字节、硬件光标控制、滚屏与简单的 C/汇编 实现示例。

## 背景与前提

- VGA 文本模式（常见 80x25）在物理内存通常映射为 0xB8000（在实模式与保护模式下均可通过实地址/物理地址访问，前提是分页不改变映射）。
- 每个字符占 2 字节：低字节为 ASCII 字符码，高字节为属性（foreground/background color + blink）。默认属性通常是 0x07（白字黑底）。
- 帧缓冲（text buffer）顺序按行优先：第一行的第一个字符位于 0xB8000，第二个字符在 0xB8002，依此类推。
- 本文示例假设 80 列 x 25 行（可配置）。

## 内存布局

- 物理地址：0xB8000
- 每行字节数：`80 columns * 2 bytes = 160 bytes`

- 总大小：`80 * 25 * 2 = 4000 bytes`

每个字符单元（2 字节）结构：

- offset +0: ASCII 字节（字符）

本文基于仓库中的 `print.c`（实现位于 `kernel/print.c`，接口声明在 `kernel/print.h`）更新了文档，详述该实现的行为、API、限制与使用示例。

实现要点（概览）

- 使用内存映射直接写入 VGA 文本缓冲区（物理地址 0xB8000）
- 屏幕尺寸：80 列 × 25 行（宏 `VGA_WIDTH` / `VGA_HEIGHT`）
- 主要 API：`putchar(char)`, `print(const char*)`, `clear_screen(void)`, `set_color(uint8_t fg, uint8_t bg)`, `printf(const char*, ...)`
- 格式化支持：`%d`/`%i`, `%u`, `%x`, `%X`, `%p`, `%c`, `%s`, `%%`
- 特殊字符：处理 `\n`, `\r`, `\t`（制表符，按 8 列边界对齐）, `\b`（退格并擦除）
- 非线程安全（游标为全局状态）

## 内存与颜色格式

- VGA 文本缓冲区物理地址：0xB8000（在多页/虚拟地址环境下必须映射到这个物理地址）
- 每个屏幕单元为 16 位：低字节=ASCII 字符，高字节=属性色（attribute）
- 当前实现使用 `current_color`（8-bit）保存当前属性，默认值 `0x07`（浅灰前景，黑色背景）
- 颜色构造器：`vga_color(fg, bg)` 返回 `fg | (bg << 4)`，`set_color` 会使用 `foreground & 0x0F` 和 `background & 0x0F`

颜色索引（0-15）含义：

- 0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta, 6=Brown, 7=LightGray
- 8=DarkGray, 9=LightBlue, 10=LightGreen, 11=LightCyan, 12=LightRed, 13=LightMagenta, 14=Yellow, 15=White

注意：实现中 `set_color(uint8_t fg, uint8_t background)` 会对参数做 `& 0x0F`，因此背景支持 0-15（高位通常用作 blink 的兼容位由 VGA 控制寄存器决定）。

## 全局状态与光标

- `vga_buffer`：指向 `uint16_t*` 的映射基址（(uint16_t*)0xB8000）
- `cursor_x`/`cursor_y`：以字符单元计的全局光标位置（列、行）
- `current_color`：当前属性字节

该实现仅更新内存中的字符与内部游标变量；它没有在 `print.c` 中包含对 VGA I/O 端口（0x3D4/0x3D5）写入光标寄存器的代码片段。如果你希望硬件光标与内存游标同步，需要在 `putchar` 或 `newline` 后调用 `vga_set_cursor`（使用 outb）或添加相应实现。

## 字符输出行为（`putchar`）

主要行为摘要：

- 普通字符：写入 `vga_buffer[index] = vga_entry(c, current_color)`，然后 `cursor_x++`。
- 当 `cursor_x >= VGA_WIDTH` 时，调用 `newline()`（即列归 0，行加 1，必要时滚屏）。
- 处理的特殊字符：
    - 处理的特殊字符：
        - `\n`：调用 `newline()`（列设 0，行 +1，超出会触发 `scroll()`）
        - `\r`：回到当前行首（`cursor_x = 0`）
        - `\t`：按 8 字符边界对齐：`cursor_x = (cursor_x + 8) & ~7`，若超出列宽，换行
        - `\b`：退格；若 `cursor_x>0`，则 `cursor_x--` 并在当前位置写入空格（擦除）

滚屏行为（`scroll()`）：

- 将第 1 行及以后的内容复制到上一行（line n+1 -> line n），然后清空最后一行（用空格和 `current_color` 填充），并把 `cursor_y` 设置为 `VGA_HEIGHT - 1`。

实现注意事项：

- `scroll()` 采用逐元素复制循环（O(VGA_WIDTH * VGA_HEIGHT)），可用 `memmove`/块复制优化性能。

## 字符串与清屏接口

- `print(const char* str)`：逐字符调用 `putchar`；当 `str == NULL` 时忽略。
- `clear_screen(void)`：把整个 `vga_buffer` 填上空格（attribute 使用 `current_color`）并把游标复位到 (0,0)。复杂度 O(VGA_SIZE)

## 颜色设置

- `set_color(uint8_t foreground, uint8_t background)`：使用 `vga_color(fg & 0x0F, bg & 0x0F)` 更新 `current_color`。

示例：

```c
set_color(15, 0); // 白字黑底
set_color(12, 4); // 亮红字，红色背景（示例）
```

## 数字与格式化（`print_number`, `printf`）

实现细节：

- `print_number(int32_t num, uint32_t base, bool is_signed, bool uppercase)`：
    - `print_number(int32_t num, uint32_t base, bool is_signed, bool uppercase)`：
        - 接受一个 32 位带符号整数 `int32_t`（内部将负数处理为 `-` 前缀，然后转换为无符号数进行位提取）
        - 支持 base 在 2..36 范围
        - 使用本地缓冲区（32 字节）将数字按位反向组装，然后从末尾输出
        - 对于 0 的特殊处理会直接输出 `'0'`

- `printf(const char* format, ...)` 支持的格式化标记：
    - `printf(const char* format, ...)` 支持的格式化标记：
        - `%d`, `%i`：带符号 32 位十进制（通过 `int32_t` 提供）
        - `%u`：无符号 32 位十进制
        - `%x`、`%X`：32 位十六进制（小写 / 大写）
        - `%p`：指针，输出 `0x` 后跟十六进制表示（实现中将 `void*` 转为 `uintptr_t` 再强转为 `int32_t` 传给 `print_number` —— 注意见下文限制）
        - `%c`：字符
        - `%s`：字符串（NULL 输出 `(null)`）
        - `%%`：输出字面 `%`

限制与注意：

- 不支持宽度、精度、长度修饰符（`%5d`, `%ld` 等）以及浮点格式（`%f`）
- `print_number` 使用 `int32_t` 做为内部数值类型：对于 64-bit 平台或指针宽度为 64 位的系统，`%p` 的实现会把 `uintptr_t` 截断到 `int32_t`，这将导致指针值错误或截断。建议在 64-bit 环境中改为以 64-bit 处理（`int64_t`/`uint64_t`）或添加专门的 `print_ptr` 实现。

示例：

```c
printf("Hello %s\n", "world");
printf("num=%d hex=0x%x ptr=%p\n", -42, 255, (void*)0xDEADBEEF);
```

## 线程安全与 SMP

当前实现是非线程安全的：`cursor_x`/`cursor_y`/`current_color` 为全局共享变量。如果在多核/多线程内核中并发调用需要加锁（例如自旋锁）以保护光标与缓冲写入操作。

建议：为 `putchar` 与 `printf` 添加一个小型自旋锁或在更高层做排他以避免并发写合并问题（尤其是多字符输出的原子性问题）。

## 64-bit 与移植警告

- `print.c` 当前实现假定 32 位整型宽度（`int32_t`）用于数字格式化和 `printf` 的整型参数。在 x86_64 或其他 64-bit 架构上需要：
    - 使用 `int64_t` / `uint64_t` 修改 `print_number`，并在 `printf` 中对 `%ld`/`%llu`/`%p` 做正确处理。
    - 小心 `va_arg` 的类型传递规则（在 64-bit 上 `long`/`pointer` 宽度可能为 8 字节）。

## 与硬件光标同步（可选）

实现中未包含写入 VGA I/O 端口来同步硬件光标的代码。如果希望同步：

```c
void vga_set_cursor_pos(uint16_t pos) {
        outb(0x3D4, 0x0F);
        outb(0x3D5, pos & 0xFF);
        outb(0x3D4, 0x0E);
        outb(0x3D5, (pos >> 8) & 0xFF);
}
```

在 `putchar` / `newline` / `scroll` 等修改 `cursor_x`/`cursor_y` 后调用 `vga_set_cursor_pos(cursor_y * VGA_WIDTH + cursor_x)` 即可使可见光标在终端上移动（前提是内核允许 I/O 端口访问）。

## 示例使用（基于当前实现）

```c
#include <kernel/print.h>

int main(void) {
        clear_screen();
        # 在 VGA 文本模式下的最小控制台与 printf（基于提供的 print.c 实现）

        本文基于仓库中的 `print.c`（实现位于 `kernel/print.c`，接口声明在 `kernel/print.h`）整理并更新了文档，详述该实现的行为、API、限制与使用示例。

        ## 实现要点（概览）

        - 使用内存映射直接写入 VGA 文本缓冲区（物理地址 0xB8000）。
        - 屏幕尺寸：80 列 × 25 行（宏 `VGA_WIDTH` / `VGA_HEIGHT`）。
        - 主要 API：`putchar(char)`, `print(const char*)`, `clear_screen(void)`, `set_color(uint8_t fg, uint8_t bg)`, `printf(const char*, ...)`。
        - 格式化支持：`%d`/`%i`, `%u`, `%x`, `%X`, `%p`, `%c`, `%s`, `%%`。
        - 特殊字符：处理 `\n`, `\r`, `\t`（制表符，按 8 列边界对齐）, `\b`（退格并擦除）。
        - 非线程安全（游标为全局状态）。

        ## 内存与颜色格式

        - VGA 文本缓冲区物理地址：`0xB8000`（在多页/虚拟地址环境下必须映射到这个物理地址）。
        - 每个屏幕单元为 16 位：低字节 = ASCII 字符，高字节 = 属性（attribute）。
        - 当前实现使用 `current_color`（8-bit）保存当前属性，默认值 `0x07`（浅灰前景，黑色背景）。
        - 颜色构造器：`vga_color(fg, bg)` 返回 `fg | (bg << 4)`，`set_color` 会使用 `foreground & 0x0F` 和 `background & 0x0F`。

        颜色索引（0-15）示例：

        - `0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta, 6=Brown, 7=LightGray`
        - `8=DarkGray, 9=LightBlue, 10=LightGreen, 11=LightCyan, 12=LightRed, 13=LightMagenta, 14=Yellow, 15=White`

        注意：实现中 `set_color(uint8_t fg, uint8_t background)` 会对参数做 `& 0x0F`，因此背景支持 0-15（高位通常用作 blink 的兼容位由 VGA 控制寄存器决定）。

        ## 全局状态与光标

        - `vga_buffer`：指向 `uint16_t*` 的映射基址（`(uint16_t*)0xB8000`）。
        - `cursor_x`/`cursor_y`：以字符单元计的全局光标位置（列、行）。
        - `current_color`：当前属性字节。

        该实现仅更新内存中的字符与内部游标变量；它没有在 `print.c` 中包含对 VGA I/O 端口（`0x3D4`/`0x3D5`）写入光标寄存器的代码片段。如果你希望硬件光标与内存游标同步，需要在 `putchar` 或 `newline` 后调用 `vga_set_cursor`（使用 `outb`）或添加相应实现。

        ## 字符输出行为（`putchar`）

        主要行为摘要：

        - 普通字符：写入 `vga_buffer[index] = vga_entry(c, current_color)`，然后 `cursor_x++`。
        - 当 `cursor_x >= VGA_WIDTH` 时，调用 `newline()`（即列归 0，行加 1，必要时滚屏）。
        - 处理的特殊字符：
            - `\n`：调用 `newline()`（列设 0，行 +1，超出会触发 `scroll()`）。
            - `\r`：回到当前行首（`cursor_x = 0`）。
            - `\t`：按 8 字符边界对齐：`cursor_x = (cursor_x + 8) & ~7`，若超出列宽，换行。
            - `\b`：退格；若 `cursor_x>0`，则 `cursor_x--` 并在当前位置写入空格（擦除）。

        滚屏行为（`scroll()`）：

        - 将第 1 行及以后的内容复制到上一行（line n+1 -> line n），然后清空最后一行（用空格和 `current_color` 填充），并把 `cursor_y` 设置为 `VGA_HEIGHT - 1`。

        实现注意事项：

        - `scroll()` 采用逐元素复制循环（O(VGA_WIDTH * VGA_HEIGHT)），可用 `memmove`/块复制优化性能。

        ## 字符串与清屏接口

        - `print(const char* str)`：逐字符调用 `putchar`；当 `str == NULL` 时忽略。
        - `clear_screen(void)`：把整个 `vga_buffer` 填上空格（attribute 使用 `current_color`）并把游标复位到 (0,0)。复杂度 O(VGA_SIZE)

        ## 颜色设置

        - `set_color(uint8_t foreground, uint8_t background)`：使用 `vga_color(fg & 0x0F, bg & 0x0F)` 更新 `current_color`。

        示例：

        ```c
        set_color(15, 0); // 白字黑底
        set_color(12, 4); // 亮红字，红色背景（示例）
        ```

        ## 数字与格式化（`print_number`, `printf`）

        实现细节：

        - `print_number(int32_t num, uint32_t base, bool is_signed, bool uppercase)`：
            - 接受一个 32 位带符号整数 `int32_t`（内部将负数处理为 `-` 前缀，然后转换为无符号数进行位提取）。
            - 支持 base 在 2..36 范围。
            - 使用本地缓冲区（32 字节）将数字按位反向组装，然后从末尾输出。
            - 对于 0 的特殊处理会直接输出 `'0'`。

        - `printf(const char* format, ...)` 支持的格式化标记：
            - `%d`, `%i`：带符号 32 位十进制（通过 `int32_t` 提供）。
            - `%u`：无符号 32 位十进制。
            - `%x`、`%X`：32 位十六进制（小写 / 大写）。
            - `%p`：指针，输出 `0x` 后跟十六进制表示（实现中将 `void*` 转为 `uintptr_t` 再强转为 `int32_t` 传给 `print_number` —— 注意见下文限制）。
            - `%c`：字符。
            - `%s`：字符串（NULL 输出 `(null)`）。
            - `%%`：输出字面 `%`。

        限制与注意：

        - 不支持宽度、精度、长度修饰符（`%5d`, `%ld` 等）以及浮点格式（`%f`）。
        - `print_number` 使用 `int32_t` 做为内部数值类型：对于 64-bit 平台或指针宽度为 64 位的系统，`%p` 的实现会把 `uintptr_t` 截断到 `int32_t`，这将导致指针值错误或截断。建议在 64-bit 环境中改为以 64-bit 处理（`int64_t`/`uint64_t`）或添加专门的 `print_ptr` 实现。

        示例：

        ```c
        printf("Hello %s\n", "world");
        printf("num=%d hex=0x%x ptr=%p\n", -42, 255, (void*)0xDEADBEEF);
        ```

        ## 线程安全与 SMP

        当前实现是非线程安全的：`cursor_x`/`cursor_y`/`current_color` 为全局共享变量。如果在多核/多线程内核中并发调用需要加锁（例如自旋锁）以保护光标与缓冲写入操作。

        建议：为 `putchar` 与 `printf` 添加一个小型自旋锁或在更高层做排他以避免并发写合并问题（尤其是多字符输出的原子性问题）。

        ## 64-bit 与移植警告

        - `print.c` 当前实现假定 32 位整型宽度（`int32_t`）用于数字格式化和 `printf` 的整型参数。在 x86_64 或其他 64-bit 架构上需要：
            - 使用 `int64_t` / `uint64_t` 修改 `print_number`，并在 `printf` 中对 `%ld`/`%llu`/`%p` 做正确处理。
            - 小心 `va_arg` 的类型传递规则（在 64-bit 上 `long`/`pointer` 宽度可能为 8 字节）。

        ## 与硬件光标同步（可选）

        实现中未包含写入 VGA I/O 端口来同步硬件光标的代码。如果希望同步：

        ```c
        void vga_set_cursor_pos(uint16_t pos) {
                outb(0x3D4, 0x0F);
                outb(0x3D5, pos & 0xFF);
                outb(0x3D4, 0x0E);
                outb(0x3D5, (pos >> 8) & 0xFF);
        }
        ```

        在 `putchar` / `newline` / `scroll` 等修改 `cursor_x`/`cursor_y` 后调用 `vga_set_cursor_pos(cursor_y * VGA_WIDTH + cursor_x)` 即可使可见光标在终端上移动（前提是内核允许 I/O 端口访问）。

        ## 示例使用（基于当前实现）

        ```c
        #include <kernel/print.h>

        int main(void) {
                clear_screen();
                set_color(15, 0); // 白字黑底
                printf("Booting...\n");
                printf("Kernel addr: %p\n", (void*)0x00100000);
                set_color(14, 4); // 黄色字，红色底
                printf("Error: %s\n", "something failed");
                return 0;
        }
        ```

        ## 建议的改进方向（可选）

        - 在 `print_number` 中支持 64-bit（`int64_t`/`uint64_t`），并在 `printf` 中对 `%ld`/`%llu`/`%p` 做正确处理。
        - 添加 `vga_set_cursor` 的实现以同步硬件光标（如果需要）。
        - 在 SMP 环境添加自旋锁保护，保证多字符输出的原子性（例如 `printf("A=%d B=%d\n", a, b)` 不会被其它 CPU 插入中断断裂）。
        - 优化 `scroll()` 使用更高效的内存复制（`memmove` / CPU-optimized memcpy）或页级移动（如果映射允许）。

        ---

        我已将文档与 `print.c` 的实现对齐并补充了限制与建议。接下来我可以：

        - 根据你要的目标平台（32-bit 或 64-bit）把 `print.c` 改为 64-bit 安全的实现并添加对应的单元/运行示例；或
        - 添加 `vga_set_cursor` 的硬件同步实现并在文档中展示如何在内核中安全调用（含 outb 实现示例）。

        请选择下一步（A: 将 `print.c` 改为 64-bit 安全并添加示例；B: 添加并启用硬件光标同步；C: 仅保留文档变更，不修改代码）。
