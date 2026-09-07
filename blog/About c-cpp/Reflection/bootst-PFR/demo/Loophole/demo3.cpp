// using type loophole to detect a struct data member
// also check the blog post http://alexpolt.github.io/type-loophole.html

#include <type_traits>

namespace A
{
    template <int N>
    struct tag
    {
    };

    template <typename T, int N>
    struct loophole_t
    {
        friend auto loophole(A::tag<N>) { return T{}; };
    };

    auto loophole(tag<0>);
}

struct detector
{
    template <class T, int = sizeof(A::loophole_t<T, 0>)>
    operator T();
};

template <typename T, int = sizeof(T{detector{}})>
constexpr int ins() { return 0; }

template <int N, typename T = decltype(loophole(A::tag<N>{}))>
T get_type();

struct test
{
    int a;
};

int main()
{
    static_assert(std::is_same<int, decltype(get_type<ins<test>()>())>::value, "");
}
