---

# `tabulate` C++ 命令行表格库详细使用文档

> 这是对 GitHub 官方仓库 [https://github.com/p-ranav/tabulate](https://github.com/p-ranav/tabulate) 内容的系统整理与翻译  
> 方便你快速掌握表格构建、格式化、访问和自定义样式等所有核心功能。

---

## 1. 简介

`tabulate` 是一个轻量级的 C++11 表格打印库，支持：

- 多行多列的 ASCII 表格打印
  
- 单元格样式（颜色、字体样式、对齐）
  
- 行列合并（row span / col span）
  
- 自定义边框符号
  
- 支持中文和 Unicode
  
- 便捷的 API 访问行、列、单元格内容
  
- 纯头文件，依赖极少
  

---

## 2. 安装与引入

### 2.1 单头文件使用

从 GitHub 下载头文件：

```bash
wget https://raw.githubusercontent.com/p-ranav/tabulate/master/include/tabulate/tabulate.hpp
```

在代码中：

```cpp
#include "tabulate/tabulate.hpp"
using namespace tabulate;
```

编译时添加 C++11 支持：

```bash
g++ -std=c++11 main.cpp -o main
```

### 2.2 CMake 集成

```cmake
add_subdirectory(path/to/tabulate)
target_link_libraries(your_target PRIVATE tabulate)
target_include_directories(your_target PRIVATE path/to/tabulate/include)
```

---

## 3. 核心类型说明

|类型名|说明|
|---|---|
|`Table`|表格对象，二维数组结构|
|`Row`|表格中的一行，是 `std::vector<Cell>`|
|`Cell`|表格单元格，包含文本和样式|
|`Format`|单元格或表格的格式设置接口|

---

## 4. 创建表格与添加数据

### 4.1 创建空表格

```cpp
Table table;
```

### 4.2 添加行

```cpp
table.add_row({"列1", "列2", "列3"});
```

参数为字符串容器，自动生成对应单元格。

### 4.3 添加单元格（追加到行尾）

```cpp
table[0].add_cell("新单元格");
```

---

## 5. 访问表格内容

### 5.1 获取行数

```cpp
size_t rows = table.size();
```

### 5.2 获取列数

通常用第一行长度作为列数参考：

```cpp
size_t cols = table[0].size();
```

### 5.3 访问单元格文本

```cpp
std::string text = table[row][col].get_text();
```

### 5.4 修改单元格文本

```cpp
table[row][col].set_text("新内容");
```

### 5.5 遍历所有内容示例

```cpp
for (size_t i = 0; i < table.size(); ++i) {
    for (size_t j = 0; j < table[i].size(); ++j) {
        std::cout << table[i][j].get_text() << " ";
    }
    std::cout << "\n";
}
```

---

## 6. 表格输出

```cpp
std::cout << table << std::endl;
```

重载了 `operator<<`，直接打印漂亮的 ASCII 表格。

---

## 7. 单元格样式设置

通过 `Cell::format()` 获得格式接口：

```cpp
table[row][col].format()
    .font_color(Color::red)          // 字体颜色
    .background_color(Color::yellow) // 背景颜色
    .font_style({FontStyle::bold})   // 加粗
    .font_align(FontAlign::center);  // 居中对齐
```

### 7.1 支持字体样式（`FontStyle`）

- `FontStyle::bold` — 粗体
  
- `FontStyle::italic` — 斜体
  
- `FontStyle::underline` — 下划线
  
- `FontStyle::crossed` — 删除线
  

### 7.2 支持颜色（`Color`）

- 标准颜色：`red`, `green`, `blue`, `yellow`, `magenta`, `cyan`, `white`, `black`
  
- 亮色版本：`bright_red`, `bright_green` 等
  

### 7.3 对齐方式（`FontAlign`）

- `left` — 左对齐
  
- `center` — 居中
  
- `right` — 右对齐
  

---

## 8. 行样式与表格样式设置

### 8.1 设置整行样式

```cpp
table[0].format()
    .font_style({FontStyle::bold})
    .font_align(FontAlign::center);
```

### 8.2 设置整张表格样式

```cpp
table.format()
    .border("─")                // 边框线
    .corner("+")                // 角落符号
    .column_separator("|")      // 列分隔符
    .font_align(FontAlign::left);
```

### 8.3 隐藏边框示例

```cpp
table.format()
    .border("")
    .corner("")
    .column_separator("");
```

---

## 9. 单元格合并

### 9.1 跨列合并（`column_span`）

```cpp
table[0][0].format().column_span(3);
```

让第一行第一列横跨3列。

### 9.2 跨行合并（`row_span`）

```cpp
table[0][0].format().row_span(2);
```

让第一行第一列纵向合并2行。

---

## 10. 隐藏单元格边框

```cpp
table[0][0].format().hide_border_top();
table[0][0].format().hide_border_bottom();
table[0][0].format().hide_border_left();
table[0][0].format().hide_border_right();
```

---

## 11. 其它常用API

|功能|方法|说明|
|---|---|---|
|清空表格|`table.clear()`|移除所有行和数据|
|获取单元格格式|`table[i][j].format()`|获取 `Format` 对象|
|设置单元格文本|`table[i][j].set_text("文本")`|修改单元格内容|
|获取单元格文本|`table[i][j].get_text()`|获取单元格字符串|
|添加新行|`table.add_row(...)`|追加一整行|
|添加新单元格|`table[row].add_cell(...)`|在指定行追加单元格|
|行数|`table.size()`|表格总行数|
|列数|`table[0].size()`|表格列数（假设所有行列数相等）|

---

## 12. 综合示例

```cpp
#include <tabulate/table.hpp>
#include <iostream>

using namespace tabulate;

int main() {
    Table table;

    // 添加标题行
    table.add_row({"ID", "名称", "得分"});
    table[0].format()
        .font_style({FontStyle::bold})
        .font_align(FontAlign::center);

    // 添加数据行
    table.add_row({"1", "张三", "88"});
    table.add_row({"2", "李四", "92"});
    table.add_row({"3", "王五", "79"});

    // 设置第二行得分字体颜色为绿色
    table[2][2].format().font_color(Color::green);

    // 设置所有得分列右对齐
    for (size_t i = 1; i < table.size(); i++) {
        table[i][2].format().font_align(FontAlign::right);
    }

    // 输出表格
    std::cout << table << std::endl;

    return 0;
}
```

---

