/// Demonstration of type loophole.
/// Friend Injection
/// By combining template instantiation with `friend` function injection, 
/// the template parameter `T` is "hidden" inside a function named `loophole(tag<N>)`,
/// and subsequently retrieved using `decltype`.
/// Compile with C++14

#include <string>
#include <type_traits>

template<int N> struct tag{};

/// 'N' is a compile-time number.
template<typename T, int N>
struct loophole_t {

    /// 这是一非成员函数，但是它的定义在类模板内部。
    /// tag<N> is compile-time index, T is the type to be injected.
    friend auto loophole(tag<N>) { 
        return T{}; 
    };
};

/// Declare the friend function to be used later.
auto loophole(tag<0>);

int main() {

    /// Instantiate the template to inject the friend function. 
    sizeof( loophole_t<std::string, 0> );   // T is std::string,N is 0, Returns std::string
    sizeof( loophole_t<int, 1> );           // T is int,N is 1, Returns int
    sizeof( loophole_t<double, 2> );        // T is double,N is 2, Returns double

    /// Mapping 
    /// tag<0> --> std::string
    /// tag<1> --> int
    /// tag<2> --> double

    /// 利用 ODR-use/实例化相关机制让 loophole_t<std::string, 0> 被实例化，从而让 friend 函数进入可查找的函数集合。
    
    return std::is_same< std::string, decltype( loophole(tag<0>{}) ) >::value;

}
