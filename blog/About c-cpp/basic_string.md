
# std::basic_string 类

在 C++ 标准库中，[`basic_string`](https://learn.microsoft.com/zh-cn/cpp/standard-library/basic-string-class?view=msvc-170) 类型专用于窄字符串和宽字符串。 字符的类型为 `std::string` 时，使用 **`char`**；字符的类型为 `std::u8string` 时，使用 **`char8_t`**；字符的类型为 `std::u16string` 时，使用 **`char16_t`**；字符的类型为 `std::u32string` 时，使用 **`char32_t`**；而字符的类型为 `std::wstring` 时，使用 **`wchar_t`**。

 由 `basic_string` 类型的一个对象控制的序列是标准 C++ 字符串类且作为字符串被引用，但不应将它们与以 null 结尾的通用于 C++ 标准库的 C 样式字符串相混淆。 标准 C++ 字符串是一个容器，它可使字符串作为普通类型使用，例如，比较和连接操作、迭代器、C++ 标准库算法以及复制由类分配器管理的内存和使用它进行分配。 如需要将标准 C++ 字符串转换为以 null 结尾的 C 样式字符串，请使用 [`basic_string::c_str`](https://learn.microsoft.com/zh-cn/cpp/standard-library/basic-string-class?view=msvc-170#c_str) 成员。
# syntax

```c
template <class CharType, 
		  class Traits    = char_traits<CharType>,
		  class Allocator = allocator<CharType>
		  >
class basic_string;
```

## CharType
要存储在字符串中的单个字符的数据类型。 C++ 标准库使用类型为 [`string`](https://learn.microsoft.com/zh-cn/cpp/standard-library/string-typedefs?view=msvc-170#string) 的元素的类型定义 `char`、类型为 [`wstring`](https://learn.microsoft.com/zh-cn/cpp/standard-library/string-typedefs?view=msvc-170#wstring) 的元素的类型定义 `wchar_t`、类型为 [`u16string`](https://learn.microsoft.com/zh-cn/cpp/standard-library/string-typedefs?view=msvc-170#u16string) 的元素的类型定义 `char16_t` 以及类型为 [`u32string`](https://learn.microsoft.com/zh-cn/cpp/standard-library/string-typedefs?view=msvc-170#u32string) 的元素的类型定义 `char32_t` 提供此类模板的专用化。
## Traits
类描述了`Traits`专用化中`basic_string`元素的各种重要属性`CharType`。 默认值为 `char_traits`<`CharType`>。
## _`Allocator`_

一种表示存储的分配器对象的类型，该分配器对象封装有关字符串的内存分配和解除分配的详细信息。 默认值为 `allocator<CharType>`