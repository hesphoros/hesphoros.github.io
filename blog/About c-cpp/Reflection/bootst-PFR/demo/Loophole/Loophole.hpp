/// 
/// Compile with C++14 standard
/// This file is a demonstration program for state meta programming using friend injection.
/// Author： hesphoros@gmail.com
/// Copyright (c) 2017-2018 Alexandr Poltavsky
/// Copyright (c) 2019-2026 Antony Polukhin.
/// Copyright (c) 2026-present hesphoros

/// Description:
/// The Great Type Loophole is a technique that allows to exchange type information with template
/// instantiations. Basically you can assign and read type information during compile time.
/// Here it is used to detect data members of a data type. I described it for the first time in
/// this blog post http://alexpolt.github.io/type-loophole.html

//
// This technique exploits the http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#2118
// CWG 2118. Stateful metaprogramming via friend injection
// Note: CWG agreed that such techniques should be ill-formed, although the mechanism for prohibiting them is as yet undetermined.

#include <type_traits>