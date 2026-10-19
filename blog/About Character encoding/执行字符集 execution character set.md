execution character set

简单理解：

> **执行字符集 = 编译器把 C/C++ 源代码里的普通字符串/字符字面量，最终转换成程序运行时使用的 `char` 字节序列时所采用的字符集。**

例如：

```cpp
const char* text = "你好";
```

这里其实经历了两个阶段：

```
源代码文件
   │
   │  源文件编码
   ▼
"你好"
   │
   │  编译器转换
   ▼
char 字节序列
   │
   │  执行字符集
   ▼
程序运行
```

### 举个 Windows/MSVC 的例子

假设你的 `.cpp` 文件保存为 UTF-8：

```cpp
const char* text = "你好";
```

如果 MSVC 使用 UTF-8 执行字符集，那么：

```cpp
text
↓
E4 BD A0 E5 A5 BD
```

也就是 UTF-8。

但是如果执行字符集是 GBK，那么它可能变成：

```
C4 E3 BA C3
```

也就是 GBK。

所以：

```
strlen(text)
```

看到的只是这些字节，并不知道它们是 UTF-8 还是 GBK。

---

### `#pragma execution_character_set("utf-8")` 

它告诉 MSVC：

> **把源代码里的普通字符/字符串字面量按照 UTF-8 作为执行字符集。**

例如：

```
#pragma execution_character_set("utf-8")

const char* text = "你好";
```

于是：

```
"你好"
     ↓
UTF-8
     ↓
E4 BD A0 E5 A5 BD
```

---

### 和“源文件编码”有什么区别？

这是最容易混淆的地方。

假设：

```
hello.cpp
```

本身保存成 UTF-8：

```
文件编码 = UTF-8
```

然后：

```
const char* str = "你好";
```

编译器首先要**读取 UTF-8 文件**，然后还要决定：

> `"你好"` 最终生成什么编码的 `char`？

所以存在两个概念：

|概念|作用|
|---|---|
|Source charset|编译器怎么读取 `.cpp`|
|Execution charset|`"你好"` 最终变成什么 `char`|
|Runtime encoding|程序运行时你认为这些 `char` 是什么编码|

例如：

```
.cpp 文件
UTF-8
  │
  │ Source Charset
  ▼
编译器理解 "你好"
  │
  │ Execution Charset
  ▼
char[]
UTF-8 / GBK / ...
```

---
