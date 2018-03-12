# constexpr-format

Compile-time printf-like string formatting in C++17

[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

## Motivation

While having fun with constexpr computations, the idea was handed to me to compose a diagram of the results in a string at compile-time using a printf-like interface.

## Example

```c++
#include "constexpr_format.hpp"

using namespace constexpr_format::string_udl;
constexpr auto format = "Hello %%%s%%, this is number %d and %d"_sv;
constexpr auto string = constexpr_format::format([]{return format;}, []{return std::tuple{"USER"_sv,1,5};});
static_assert(string == "Hello %USER%, this is number 1 and 5");
```

constexpr_format::string_udl is a namespace with one user-defined literal: _sv, which returns an internal constexpr implementation of string_view.
The arguments to constexpr_format::format are constexpr lambda's, which, using this constexpr lambda idiom, allows us to pass arbitrary literal values to constexpr functions as constexpr.

## Features

### Supported format specifiers
 - %d, accepts any type convertible to int
 - %%, prints out a %
 - %s, 

### (Relatively) readable compilation errors for incorrect arguments

Giving too few or too many arguments:
```
<source>: In instantiation of 'constexpr bool constexpr_format::format_string::detail::check_format(F, Tup) [with F = constexpr_format::format_parser::FormatString<constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2> >; Tup = std::tuple<constexpr_format::util::string_view, int>]':
<source>:322:42:   required from 'constexpr auto constexpr_format::format_string::detail::format_impl(FormatF, ArgTupF) [with FormatF = constexpr_format::format_string::format(StringOrFormatF, TupF) [with StringOrFormatF = test()::<lambda()>; TupF = test()::<lambda()>]::<lambda()>; ArgTupF = test()::<lambda()>]'
<source>:359:43:   required from 'constexpr auto constexpr_format::format_string::format(StringOrFormatF, TupF) [with StringOrFormatF = test()::<lambda()>; TupF = test()::<lambda()>]'
<source>:380:148:   required from here
<source>:305:42: error: static assertion failed: Too few arguments for format
                 static_assert(tuple_size >= num_args, "Too few arguments for format");
                               ~~~~~~~~~~~^~~~~~~~~~~
Compiler returned: 1
```

Wrong type, here I gave a string_view as the second argument where the format expected an int.
```
<source>: In instantiation of 'constexpr bool constexpr_format::format_string::detail::check_format_conversion(FormatSpec, Tup) [with FormatSpec = constexpr_format::format_parser::FormatSpec<int, 1>; Tup = std::tuple<constexpr_format::util::string_view, constexpr_format::util::string_view, int>]':
<source>:312:56:   required from 'constexpr_format::format_string::detail::check_format(F, Tup) [with F = constexpr_format::format_parser::FormatString<constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2> >; Tup = std::tuple<constexpr_format::util::string_view, constexpr_format::util::string_view, int>]::<lambda(auto:5 ...)> [with auto:5 = {constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2>}]'
<source>:222:25:   required from 'static constexpr auto constexpr_format::format_parser::FormatString<FormatSpecs>::apply(F) [with F = constexpr_format::format_string::detail::check_format(F, Tup) [with F = constexpr_format::format_parser::FormatString<constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2> >; Tup = std::tuple<constexpr_format::util::string_view, constexpr_format::util::string_view, int>]::<lambda(auto:5 ...)>; FormatSpecs = {constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2>}]'
<source>:311:45:   required from 'constexpr bool constexpr_format::format_string::detail::check_format(F, Tup) [with F = constexpr_format::format_parser::FormatString<constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<constexpr_format::util::string_view, 0>, constexpr_format::format_parser::FormatSpec<constexpr_format::LiteralPercent, -1>, constexpr_format::format_parser::FormatSpec<int, 1>, constexpr_format::format_parser::FormatSpec<int, 2> >; Tup = std::tuple<constexpr_format::util::string_view, constexpr_format::util::string_view, int>]'
<source>:322:42:   required from 'constexpr auto constexpr_format::format_string::detail::format_impl(FormatF, ArgTupF) [with FormatF = constexpr_format::format_string::format(StringOrFormatF, TupF) [with StringOrFormatF = test()::<lambda()>; TupF = test()::<lambda()>]::<lambda()>; ArgTupF = test()::<lambda()>]'
<source>:359:43:   required from 'constexpr auto constexpr_format::format_string::format(StringOrFormatF, TupF) [with StringOrFormatF = test()::<lambda()>; TupF = test()::<lambda()>]'
<source>:380:156:   required from here
<source>:291:35: error: static assertion failed: Mismatched format types
                     static_assert(std::is_convertible_v<U,T>, "Mismatched format types");
                                   ^~~
Compiler returned: 1
```

## Core implementation details

### Constexpr lambda parameters
In the first lambda in our example above, format doesn't need to be captured because it isn't odr-used, hence allowing this idiom to exist. This makes much of the code implementing the constexpr format
pretty straightforward in comparison to the TMP trickery that would be necessary to achieve this using variadic templates.

### util::static_string - util::string_view
These two types form the core of the string processing in this library. 
string_view provides constexpr views over string literals(or compile-time strings), primarily used in parsing the format string.
static_string<N> is a light wrapper around std::array<char,N> used for building up the result of format.

### Format specifiers
Format specifiers are added by adding a declaration of a function called to_type in the constexpr_format::format_to_type namespace, taking a template character wrapper type and returning a template instantiation of format_to_type::FormatType.
The function is only ever used in a decltype context, so no implementation is necessary.
```c++
namespace constexpr_format::format_to_type {
template<typename T, bool takesParam = true>
struct FormatType {
    using type = T;
    constexpr static bool hasParam = takesParam;
};

template<char C>
struct CharV {};

auto to_type(CharV<'d'>) -> FormatType<int>;
auto to_type(CharV<'s'>) -> FormatType<util::string_view>;
auto to_type(CharV<'%'>) -> FormatType<LiteralPercent,false>;
}
```

With T being the type expected by the formatter, and takesParam a boolean saying whether this specifier consumes an argument given to format(eg: "%%" translates to "%").

Each formatter is a specialization of the constexpr_format::Format template. If it doesn't take a parameter, it has one constexpr static non-templated method: get_string, taking no parameters and returning a value convertible to a util::static_string.
If it does take a parameter, it has two methods:
 - constexpr static std::size_t length(T), taking a value of the formatted type and returns the length of the string it will generate
 - template<std::size_t N> constexpr static util::static_string<N> get_string(T), which builds the string


## Compiler support

Currently the only compiler this project compiles with is gcc 8.0+. This is because of the constexpr lambda idiom used throughout, which reveals some compiler bugs.

All current versions of clang do not allow constexpr values of user-defined literal types to be non-odr-used in lambda's without being captured:
```c++
void testNativeLiteralType() {
    //Compiles fine
    constexpr int n = 0;
    auto D = []{return n;};
    constexpr auto x = D();
}

//Obviously literal type
struct A {
    int a = 0;
};

void testNonNativeLiteralType() {
    //Doesn't compile on clang
    constexpr auto n = A{};
    auto D = []{return n;};
    constexpr auto x = D();
}
```

While gcc 7.3 doesn't allow lambda expressions as part of a parameter pack expansion:
```c++
template<typename... Fs>
void run(Fs... fs) {
    (fs(),...);
}

template<typename... T>
void f(T... x) {
    run([=]{return x;}...);
}

void test() {
    f(1,2,3,'a');
}
```

## Roadmap

- [ ] Better compile-time errors for format string parsing errors
- [ ] Add more complete format specifiers support
  - [ ] floating-point
- [ ] Add parser/serializer support for format options(ie: "%04d")
- [ ] Find a better way of inserting new formatters.
