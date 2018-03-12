#include "constexpr_format.hpp"

void test() {
    using namespace constexpr_format::string_udl;
    constexpr static auto s = constexpr_format::format([]{return "Hello %%%s%%, this is number %d and %d"_sv;}, []{return std::tuple{"USER"_sv,1,5};});
    static_assert(s == "Hello %USER%, this is number 1 and 5");
}
