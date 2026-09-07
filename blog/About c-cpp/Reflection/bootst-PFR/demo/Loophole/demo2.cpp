/// From :  https://coliru.stacked-crooked.com/a/6094c5aa5e75e240
/// Author : https://github.com/sasha-s
/// Assign a unique ID to the type at compile time.

/// Compile with: g++ -std=c++14 -O2 -Wall -pedantic -pthread main.cpp && ./a.out
/**
 *  output:
 *  tid<char>: 0
 *  tid<int>: 1
 *  tid<void>: 2
 *  tid<int>: 1
 *  tid<string>: 3
 *  a:123
 *  _1:1
 *  tid<string, int>: 0
 *  tid<void, int>: 1
 * 
*/
#include <type_traits>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"

template <typename TAG, int N>
struct tag {
    friend auto loophole(tag<TAG, N>);

    // This is a constexpr friend function that returns an int.
    // This function is used to check if a type has been assigned an ID.
    constexpr friend int cloophole(tag<TAG, N>);
};

/*
    The definitions of friend functions.
*/
template <typename TAG, typename U, int N, bool B>
struct fn_def {
    friend auto loophole(tag<TAG, N>) { return U{}; }
    constexpr friend int cloophole(tag<TAG, N>) { return 0; }
};

/*
    This specialization is to avoid multiple definition errors.
*/
template <typename TAG, typename U, int N>
struct fn_def<TAG, U, N, true> {};

/*
    This has a templated conversion operator which in turn triggers instantiations.
    Important point, using sizeof seems to be more reliable. Also default template
    arguments are "cached" (I think). To fix that I provide a U template parameter to
    the ins functions which do the detection using constexpr friend functions and SFINAE.
*/
template <typename TAG, int N>
struct c_op {


    template <typename U, int M>
    static auto ins(...) -> int;

    /// int = cloophole(tag<TAG, M>{}) is a default template argument
    /// that will be used to check if the type has been assigned an ID.
    /// If `cloophole(tag<TAG, M>{})` is don't exist, use the second overload of `ins` which returns int.
    template <typename U, int M, int = cloophole(tag<TAG, M>{})>
    static auto ins(int) -> char;

    /// Is 'cloophole(tag<TAG, M>{})' a valid expression?
    template <typename U, int = sizeof(fn_def<TAG, U, N, sizeof(ins<U, N>(0)) == sizeof(char)>)>
    /// Conversion function template to U* type. 
    operator U*();
};



/// @param T The type to be assigned an ID.
/// @param TAG A tag type to differentiate between different ID spaces. This allows the same type to have different IDs in different contexts.
/// @param N The current ID to be assigned.Start from 0 and increment for each new type.
/// @param b A boolean value to indicate if the type has been assigned an ID.
///        ins<TAG, N>(0) will return char if the type has been assigned an ID, otherwise it will return int.
template <typename T, typename TAG = void, int N = 0,
        bool b = (sizeof(c_op<TAG, N>::template ins<T, N>(0)) == sizeof(char))>
struct tid {
    const static int value = tid<T, TAG, N + 1>::value;
};

/// N is not used
template <typename T, typename TAG, int N>
struct tid<T, TAG, N, false> {
    /// This is a typedef that uses the conversion operator of c_op<TAG, N> to convert to T* type. -> `operator U*()`
    typedef decltype(static_cast<T*>(c_op<TAG, N>{}), 0) X;
    const static int value = N;
};

/// T -> tid<T>         -> N
/// N -> type_by_id<N>  -> T
template <int N, typename TAG=void>
struct type_by_id {
    typedef decltype(loophole(tag<TAG, N>())) type;
};
#pragma GCC diagnostic pop

#include <iostream>
#include <string>

int main() {

    std::cout << "tid<char>: "      << tid<char>::value << std::endl;
    std::cout << "tid<int>: "       << tid<int>::value << std::endl;
    std::cout << "tid<void>: "      << tid<void>::value << std::endl;
    std::cout << "tid<int>: "       << tid<int>::value << std::endl;
    std::cout << "tid<string>: "    << tid<std::string>::value << std::endl;
    type_by_id<3>::type a = "123";
    type_by_id<1>::type _1 = 1;
    std::cout << "a:"               << a << std::endl;
    std::cout << "_1:"              << _1 << std::endl;
    std::cout << "tid<string, int>: " << tid<std::string, int>::value << std::endl;
    std::cout << "tid<void, int>: " << tid<void, int>::value << std::endl;

    return 0;
}
