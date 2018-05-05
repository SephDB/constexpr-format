#include <array>
#include <tuple>
#include <algorithm>

namespace constexpr_format {

    //Utility data structures and functions
    namespace util {

        //Wrapper for std::array<char,N> to not overload operator+ on std::array for anyone using this namespace.
        template<std::size_t N>
        struct static_string {
            std::array<char,N> string;

            constexpr decltype(auto) operator[](std::size_t n) {return string[n];}
            constexpr decltype(auto) operator[](std::size_t n) const {return string[n];}

            constexpr decltype(auto) data() {return string.data();}
            constexpr decltype(auto) data() const {return string.data();}

            constexpr auto size() const {return N;}

            constexpr auto begin() {return string.begin();}
            constexpr auto begin() const {return string.begin();}

            constexpr auto end() {return string.end();}
            constexpr auto end() const {return string.end();}

            constexpr std::array<char,N+1> getNullTerminatedString() {
                return (*this + static_string<1>{{'\0'}}).string;
            }
        };

        template<std::size_t N, std::size_t M>
        constexpr auto operator+(static_string<N> a, static_string<M> b) {
            return std::apply([&](const auto&... as) {
                return std::apply([&](const auto&... bs) {
                    return static_string<N+M>{{as...,bs...}};
                }, b.string);
            }, a.string);
        }

        //Simple constexpr-enabled string_view for views on char arrays
        class string_view {
            const char* data;
            std::size_t n;
        public:
            template<int N>
            constexpr string_view(const char (&init)[N]) : data(init),n(init[N-1]=='\0'?N-1:N) {};

            template<std::size_t N>
            constexpr string_view(const static_string<N>& array) : data(array.string.data()), n(array[N-1]=='\0'?N-1:N) {};

            constexpr string_view(const char* init, std::size_t len) : data(init),n(len) {};

            constexpr string_view(const string_view&) = default;

            constexpr std::size_t size() const {return n;};
            constexpr const char& operator[](int i) const {
                return data[i];
            }
            constexpr auto* begin() const {
                return data;
            }
            constexpr auto* end() const {
                return data+n;
            }

            constexpr std::size_t find(char c) const {
                for(std::size_t i = 0; i < n; ++i) {
                    if (data[i] == c) return i;
                }
                return n;
            }

            constexpr string_view prefix(std::size_t len) const {
                if(len >= n) return *this;
                return {data,len};
            }

            constexpr string_view remove_prefix(std::size_t len) const {
                if(len >= n) return {data,0};
                return {data+len,n-len};
            }
        };

        constexpr bool operator==(const string_view& a, const string_view& other) {
            if(a.size() != other.size()) {
                return false;
            }
            auto start = a.begin();
            for(auto c : other) {
                if (*start++ != c) return false;
            }
            return true;
        }

        template<std::size_t... I>
        constexpr auto view_to_static_impl(string_view s, std::index_sequence<I...>) {
            return static_string<sizeof...(I)>{{s[I]...}};
        }

        template<int N>
        constexpr auto view_to_static(string_view s) {
            return view_to_static_impl(s,std::make_index_sequence<N>{});
        }

        template<typename StringViewF>
        constexpr auto view_to_static(StringViewF s) {
            return view_to_static_impl(s(),std::make_index_sequence<s().size()>{});
        }

        namespace detail {
            template <class F, class TupleF, std::size_t... I>
            constexpr decltype(auto) constexpr_apply_impl(F&& f, TupleF t, std::index_sequence<I...>)
            {
                return std::forward<F>(f)([t]{return std::get<I>(t());}...);
            }
        }

        //std::apply using constexpr lambda idiom
        template <class F, class TupleF>
        constexpr decltype(auto) constexpr_apply(F&& f, TupleF t)
        {
            return detail::constexpr_apply_impl(
                std::forward<F>(f), t,
                std::make_index_sequence<std::tuple_size_v<std::remove_cv_t<decltype(t())>>>{});
        }

        template<typename T, std::size_t N>
        constexpr auto prepend(T t, std::array<T,N> a) {
            return std::apply([&](const auto&... as) {
                return std::array{t,as...};
            },a);
        }

    }

    template<char...>
    struct Literal;

    template<typename T, typename SFINAE_Check=void>
    struct Format;

    template<char... Cs>
    struct Format<Literal<Cs...>> {
        constexpr static auto get_string() {
            return util::static_string<sizeof...(Cs)>{{Cs...}};
        }
    };

    template<typename T>
    struct Format<T,std::enable_if_t<std::is_integral_v<T>>> {
        template<T N>
        constexpr static auto get_string_rec() {
            constexpr auto current = util::static_string<1>{{'0'+std::abs(N%10)}};
            if constexpr (N/10 == 0) {
                return current;
            } else {
                return get_string_rec<N/10>()+current;
            }
        }
        template<typename IntF>
        constexpr static auto get_string(IntF f) {
            constexpr T N = f();
            if constexpr (N == 0) {
                return util::static_string<1>{{'0'}};
            } else {
                constexpr auto abs_num = get_string_rec<N>();
                if constexpr (N < 0) {
                    return util::static_string<1>{{'-'}}+abs_num;
                } else {
                    return abs_num;
                }
            }
        }
    };

    template<>
    struct Format<util::string_view> {
        template<typename StringF>
        constexpr static auto get_string(StringF f) {
            constexpr auto s = f();
            return util::view_to_static<s.size()>(s);
        };
    };

    namespace format_to_typecheck {
        template<typename T>
        struct Id {
            template<typename U>
            static constexpr bool check() {
                constexpr bool val = std::is_same_v<T,U>;
                static_assert(val,"Incompatible types");
                return val;
            }
        };
        template<template<typename> class Check>
        struct TypeCheck {
            template<typename U>
            static constexpr bool check() {
                constexpr bool val = Check<U>::value;
                static_assert(val,"Incompatible type");
                return val;
            };
        };
        struct Any {
            template<typename U>
            static constexpr bool check() {
                return true;
            }
        };

        template<char C>
        struct CharV {};

        auto to_type(CharV<'d'>) -> TypeCheck<std::is_integral>;
        auto to_type(CharV<'s'>) -> Id<util::string_view>;
    }

    namespace format_parser {

        template<char... Cs>
        struct ParsingMode {
            constexpr static int find_first(util::string_view s) {
                return std::min({s.find(Cs)...});
            };
        };

        using PythonFmt = ParsingMode<'{','}'>;
        using PrintfFmt = ParsingMode<'%'>;

        //To be filled in later
        struct FormatOptions {};

        template<typename T, int ParamNum>
        struct FormatSpec {
            using type = T;
            constexpr static int num = ParamNum;
        };

        template<typename FmtSpec>
        struct Spec {
            using spec = FmtSpec;
            FormatOptions opts;
            util::string_view suffix;
            int next_index;
        };


        template<int current, typename StringF>
        constexpr auto parse_spec_dispatch(StringF fs, PythonFmt) {
            //TODO
        }

        template<int currentParam, typename StringF>
        constexpr auto parse_spec_dispatch(StringF fs, PrintfFmt) {
            constexpr auto s = fs();

            using namespace format_to_typecheck;
            using FormatSpecT = FormatSpec<decltype(to_type(CharV<s[1]>{})), currentParam>;

            return Spec<FormatSpecT>{{},s.remove_prefix(2),currentParam+1};
        }

        template<int currentParam, typename StringF, typename Mode>
        constexpr auto parse_spec(StringF fs, Mode m) {
            if constexpr (fs()[0] == fs()[1]) {
                //Repeated special character, insert literal
                using FormatSpecT = FormatSpec<Literal<fs()[0]>,-1>;
                return Spec<FormatSpecT>{{},fs().remove_prefix(2),currentParam};
            } else {
                return parse_spec_dispatch<currentParam>(fs,m);
            }
        }

        template<typename... FormatSpecs>
        struct FormatString {
            std::array<util::string_view,sizeof...(FormatSpecs)+1> strings;
            std::array<FormatOptions,sizeof...(FormatSpecs)> options;
            template<typename F>
            constexpr static auto apply(F f) {
                return f(FormatSpecs{}...);
            }
        };

        namespace detail {
            template<typename FSpec, typename FResult>
            struct getFResult {};

            template<typename FSpec, typename... FSpecs>
            struct getFResult<FSpec, FormatString<FSpecs...>> {
                using type = FormatString<FSpec,FSpecs...>;
            };

            template<typename FSpec, typename FResult>
            using FormatResult_t = typename getFResult<FSpec,FResult>::type;

        }

        template<int currentParam,typename Mode,typename StringViewF>
        constexpr auto parse_format_impl(StringViewF format) {
            constexpr util::string_view f = format();
            constexpr auto format_index = Mode::find_first(f);
            if constexpr (f.size() == format_index) {
                return FormatString<>{{f},{}};
            } else {
                constexpr auto prefix = f.prefix(format_index);

                constexpr auto spec_suffix = f.remove_prefix(format_index);
                constexpr auto spec = parse_spec<currentParam>([]{return spec_suffix;},Mode{});

                constexpr auto suffix = spec.suffix;

                using FormatSpecT = typename decltype(spec)::spec;

                constexpr auto result = parse_format_impl<spec.next_index,Mode>([]{return suffix;});

                return detail::FormatResult_t<FormatSpecT,std::remove_cv_t<decltype(result)>>{
                    util::prepend(prefix,result.strings),
                    util::prepend(FormatOptions{},result.options)
                };
            }
        }

        template<typename StringViewF>
        constexpr auto parse_format(StringViewF format) {
            return parse_format_impl<0,PrintfFmt>(format);
        }

    }

    namespace format_string {

        namespace detail {

            template<typename T>
            struct is_format {
                static constexpr bool value = false;
            };

            template<typename... T>
            struct is_format<format_parser::FormatString<T...>> {
                static constexpr bool value = true;
            };

            template<typename FormatSpec, typename Tup>
            constexpr bool check_format_conversion(FormatSpec,Tup t) {
                if constexpr(FormatSpec::num != -1) {
                    using T = typename FormatSpec::type;
                    using U = std::decay_t<decltype(std::get<FormatSpec::num>(t))>;
                    constexpr bool check_result = T::template check<U>();
                    static_assert(check_result, "Mismatched format types");
                    return check_result;
                } else {
                    return true;
                }
            }

            template<typename F, typename Tup>
            constexpr bool check_format(F,Tup t) {
                constexpr auto num_args = F::template apply([](auto... fs) {
                    return ((fs.num != -1) + ... + 0);
                });
                constexpr auto tuple_size = std::tuple_size_v<Tup>;
                static_assert(tuple_size <= num_args, "Too many arguments for format");
                static_assert(tuple_size >= num_args, "Too few arguments for format");

                if constexpr(tuple_size != num_args) {
                    //Error case still gives reasonable type to reduce compilation error output
                    return false;
                } else {
                    return F::template apply([=](auto... formats) {
                        return (check_format_conversion(formats,t) && ...);
                    });
                }
            }

            template<typename FormatF, typename ArgTupF>
            constexpr auto format_impl(FormatF format, ArgTupF argsf) {
                constexpr auto f = format();
                constexpr std::tuple args = argsf();

                if constexpr(check_format(f,args)) {
                    constexpr auto prefix = f.strings[0];
                    constexpr auto init = util::view_to_static([]{return prefix;});

                    constexpr auto intermediate_strings = std::apply([](auto first, auto... xs) {
                        return std::array<util::string_view,sizeof...(xs)>{xs...};
                    },f.strings);
                    constexpr auto options = f.options;

                    //Zip results of formatting with strings in-between
                    constexpr auto getString = [](auto format) constexpr {
                        //If the format doesn't consume a parameter, it has a num of -1
                        if constexpr(decltype(format)::num == -1) {
                            using type = typename decltype(format)::type;
                            return Format<type>::get_string();
                        } else {
                            constexpr auto val = std::get<format.num>(args);
                            return Format<std::decay_t<decltype(val)>>::template get_string([]{return val;});
                        }
                    };
                    return init+f.template apply([=](auto... fs) {
                        return util::constexpr_apply([=](auto... prefixes) {
                            return ((getString(fs) + util::view_to_static(prefixes)) + ... + util::static_string<0>{});
                        },[]{return intermediate_strings;});
                    });
                } else {
                    //Error case still gives reasonable type to reduce compilation error output
                    return util::static_string<1>{{'\0'}};
                }
            }
        }

        template<typename StringOrFormatF, typename TupF>
        constexpr auto format(StringOrFormatF format, TupF tup) {
            if constexpr (std::is_convertible_v<decltype(format()),util::string_view>) {
                constexpr auto f = format_parser::parse_format(format);
                return detail::format_impl([]{return f;},tup);
            } else {
                return detail::format_impl(format,tup);
            }
        }
    }

    using format_parser::parse_format;
    using format_string::format;

    namespace string_udl {
        constexpr auto operator""_sv (const char* c, std::size_t n) {
            return util::string_view(c,n);
        }
    }

}