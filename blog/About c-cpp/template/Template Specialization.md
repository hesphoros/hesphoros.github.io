# 省略号和可变参数模板

“可变参数模板”是支持任意数量的参数的类或函数模板。 此机制对 C++ 库开发人员特别有用：可以将其应用于类模板和函数模板，从而提供广泛的类型安全且非凡的功能和灵活性。

# 可变参数类模板

## 语法
两种模板都可以使用sizeof() 来打印参数个数
```c++
template <class... T>
void f(T... args)
{    
    cout << sizeof...(args) << endl; //打印变参的个数
}

f();        //0
f(1, 2);    //2
f(1, 2.5, "");    //3
```

可变参数模板以两种方式使用省略号。 

- 在参数名称的左侧，表示“参数包”
- 在参数名称的右侧，将参数包扩展为单独的名称。

```c++
template<typename... Arguments> class classname;
```

也可以根据自己的偏好在省略号周围添加空白

```c++
template<typename ...Arguments> class classname;
template<typename ... Arguments> class classname;
```

以上示例中 `Argument`是参数包 ,类 `classname` 可以接受可变数量的参数，如以下示例所示：

```c++
template<typename... Arguments> class vtclass;

vtclass< > vtinstance1;
vtclass<int> vtinstance2;
vtclass<float, bool> vtinstance3;
vtclass<long, std::vector<int>, std::string> vtinstance4;
```

通过使用可变参数类模板定义，还可以至少需要一个参数：

```c++
template <typename First, typename... Rest> class classname;
```

# 可变参数函数模板

## 语法

```c++

template <typename... Arguments> returntype functionname(Arguments... args);
```

也可以使用如下形式

```c++
template <typename... Arguments> returntype functionname(Arguments&... args);
template <typename... Arguments> returntype functionname(Arguments&&... args);
template <typename... Arguments> returntype functionname(Arguments*... args);
```

也允许使用说明符

```c++
template <typename... Arguments> returntype functionname(const Arguments&... args);
```

与可变参数模板类定义一样，可以生成要求至少一个参数的函数：

```c++
template <typename First, typename... Rest> returntype functionname(const First& first, const Rest&... args);
```

# 易混淆

- 在模板参数列表 (`template <parameter-list>`) 中，`typename...` 引入了模板参数包。

- 在参数声明子句 (`func(parameter-list)`) 中，“顶级”省略号引入了函数参数包，并且省略号定位很重要：

  C++

  ```cpp
  // v1 is NOT a function parameter pack:
  template <typename... Types> void func1(std::vector<Types...> v1);
  
  // v2 IS a function parameter pack:
  template <typename... Types> void func2(std::vector<Types>... v2);
  ```

- 其中省略号显示在参数名称的后面，并且您具有一个参数包扩展。