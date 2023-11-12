#ifndef __TJH_FORMAT2_HPP__
#define __TJH_FORMAT2_HPP__

#include<string>
#include<sstream>
#include<iomanip>
#include<iostream>
#include<array>

#include "compileTimeString2.hpp"

#if __cplusplus < 201400
#error "c++ standard must >= c++14/gnu++14"
#endif

namespace fmt 
{
using Char = cts::Char;
template<Char ...Cs>
using Cts = cts::Str<Cs...>;

//使用Fmtss，获取格式化后的std::stringstream
template<Char ...Cs, typename ...A>
constexpr std::stringstream Fmtss(const Cts<Cs...> &fmt, A &&...a);
#define FMTSS(__fmt, ...) \
    do { \
        DEF_CTS(); \
        fmt::Fmtss(CTS(__fmt), __VA_ARGS__); \
    } while(0)

//使用Fmts，获取格式化后的std::string
template<Char ...Cs, typename ...A>
constexpr std::string Fmts(const Cts<Cs...> &fmt, A &&...a);
#define FMTS(__fmt, ...) \
    do { \
        DEF_CTS(); \
        fmt::Fmts(CTS(__fmt), __VA_ARGS__); \
    } while(0)

//使用Fmt，获取打印格式化后的字符串，并返回打印内容的长度
template<Char ...Cs, typename ...A>
constexpr size_t Fmt(const Cts<Cs...> &fmt, A &&...a);
#define FMT(__fmt, ...) \
    do { \
        DEF_CTS(); \
        fmt::Fmt(CTS(__fmt), __VA_ARGS__); \
    } while(0)

namespace __detail 
{
template<Char C>
constexpr bool IsNum() { return '0' <= C && C <= '9'; }

/*********** 从Cts中获取一个数值：遇到第一个非数值字符就停止 ************/
template<Char C, Char ...Cs, 
    std::enable_if_t<(!IsNum<C>()), int> = 0>
constexpr ssize_t GetNum(const Cts<C, Cs...> &s) {
    return -1;
}
template<Char C, Char ...Cs, 
    std::enable_if_t<(IsNum<C>() && sizeof...(Cs) == 0), int> = 0>
constexpr ssize_t GetNum(const Cts<C, Cs...> &s) {
    return C - '0';
}
template<Char C, Char ...Cs, 
    std::enable_if_t<(IsNum<C>() && sizeof...(Cs) > 0), int> = 0>
constexpr ssize_t GetNum(const Cts<C, Cs...> &) {
    //不能使用参数列表的东西，比如如果参数列表命名为fmt
    //也即const Cts<Cs...> &s，会因为s不是常量，
    //  所以不能使用一些常量函数
    constexpr Cts<C, Cs...> s;
    constexpr ssize_t next = GetNum(s.template RSubStr<1>());
    if(next > 0) {
        return (C - '0') * 10 + next;
    } else {
        return (C - '0');
    }
}
struct FmtFlag {
    //if fmt contains 'w'，这里要单独列出来是因为有一个static_assert，
    //  如果写成if-else，则如果没有'w'时总是会导致static_assert失败（c++17才有if constexpr）
    template<Char ...Cs, 
        std::enable_if_t<Cts<Cs...>::template Contains<'w'>(), int> = 0>
    static constexpr void GetW(FmtFlag &flag) {
        constexpr Cts<Cs...> fmt; 
        constexpr auto idx = fmt.Find(Cts<'w'>{});
        static_assert(idx < fmt.Len(), 
            "w must be followed by a num!");
        constexpr auto width = GetNum(fmt.template RSubStr<idx + 1>());
        flag.width = width < 0 ? 0 : width;
    }
    template<Char ...Cs, 
        std::enable_if_t<!Cts<Cs...>::template Contains<'w'>(), int> = 0>
    static constexpr void GetW(FmtFlag &flag) {
        flag.width = 0;
    }
    //if fmt contains 'f'，这里要单独列出来是因为有一个static_assert，
    //  如果写成if-else，则如果没有'f'时总是会导致static_assert失败（c++17才有if constexpr）
    template<Char ...Cs, 
        std::enable_if_t<Cts<Cs...>::template Contains<'f'>(), int> = 0>
    static constexpr void GetF(FmtFlag &flag) {
        constexpr Cts<Cs...> fmt; 
        constexpr auto idx = fmt.Find(Cts<'f'>{});
        static_assert(idx < fmt.Len() - 1, 
            "f must be followed by a char!");
        flag.fill = fmt[idx + 1];
    }
    template<Char ...Cs, 
        std::enable_if_t<!Cts<Cs...>::template Contains<'f'>(), int> = 0>
    static constexpr void GetF(FmtFlag &flag) {
        flag.fill = ' ';
    }
    //根据传入的Cts，获取特定的fmt flag
    template<Char ...Cs>
    static constexpr FmtFlag Get(const Cts<Cs...> &) {
        FmtFlag ret;
        //不能使用参数列表的东西，比如如果参数列表命名为fmt
        //也即const Cts<Cs...> &fmt，会因为fmt不是常量，
        //  所以不能使用一些常量函数
        constexpr Cts<Cs...> fmt; 
        //单独处理'w'项
        GetW<Cs...>(ret);
        if(fmt.template Contains<'h', 'H'>()) {
            ret.hex = fmt.template Contains<'h'>() ? 1 : 2;
        } else {
            ret.hex = 0;
        }
        ret.left = fmt.template Contains<'l'>();
        GetF<Cs...>(ret);
        return ret;
    }
    //operator<< 类似于std::boolalpha的效果，改变流的形式
    friend std::stringstream &operator<<(std::stringstream &ss, const FmtFlag &f)
    {
        if(f.width > 0) {
            ss << std::setw(f.width);
        }
        if(f.hex > 0) {
            ss << std::hex;
            if(f.hex == 2) {
                ss << std::uppercase;
            }
        }
        if(f.left) {
            ss << std::left;
        } else {
            ss << std::right;
        }
        if(f.fill != ' ') {
            ss << std::setfill(f.fill);
        }
        return ss;
    }
  public:
    size_t width = 0;
    int hex = 0;    //0 -- no hex, 1 -- hex, 2 -- Hex
    bool left = true;
    char fill = ' ';
};

//获取Fmt相关信息的状态机
enum class FmtState {
    Normal, //一般状态
    Percent, //最近遇到了'%'
    Brace, //最近遇到了'{'
};
//获取fmt中的占位符个数{xxx}，其中xxx是fmt的格式化标志
//%% 转义为 % 
//%{ 转义为 {
template<FmtState State, Char ...Cs>
struct GetFmtSize;
template<Char ...Cs>
struct GetFmtSize<FmtState::Normal, Cs...> {
    template<size_t Idx,
        std::enable_if_t<(Idx == sizeof...(Cs)), int> = 0>
    static constexpr size_t fn() {
        return 0;
    }
    template<size_t Idx,
        std::enable_if_t<(Idx < sizeof...(Cs)), int> = 0>
    static constexpr size_t fn() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '{') {
            return GetFmtSize<FmtState::Brace, Cs...>::
                template fn<Idx + 1>();
        } else {
            return GetFmtSize<FmtState::Normal, Cs...>::
                template fn<Idx + 1>();
        }
    }
};
template<Char ...Cs>
struct GetFmtSize<FmtState::Brace, Cs...> {
    template<size_t Idx,
        std::enable_if_t<(Idx == sizeof...(Cs)), int> = 0>
    static constexpr size_t fn() {
        return 0;
    }
    template<size_t Idx,
        std::enable_if_t<(Idx < sizeof...(Cs)), int> = 0>
    static constexpr size_t fn() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '}') {
            return 1 + GetFmtSize<FmtState::Normal, Cs...>::
                template fn<Idx + 1>();
        } else {
            return GetFmtSize<FmtState::Brace, Cs...>::
                template fn<Idx + 1>();
        }
    }
};

//使用index获取fmt的具体位置
template<FmtState State, Char ...Cs>
struct GetFmtByIndex;
template<Char ...Cs>
struct GetFmtByIndex<FmtState::Normal, Cs...> {
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        return -1;
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal > 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '{') {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template begin<Goal, Idx + 1>();
        } else {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template begin<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal == 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '{') {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template begin<Goal, Idx + 1>() > 0 ? Idx : -1;
        } else {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template begin<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        return -1;
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal > 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '{') {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template end<Goal, Idx + 1>();
        } else {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template end<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal == 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '{') {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template end<Goal, Idx + 1>();
        } else {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template end<Goal, Idx + 1>();
        }
    }
};
template<Char ...Cs>
struct GetFmtByIndex<FmtState::Brace, Cs...> {
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        return -1;
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal > 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '}') {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template begin<Goal - 1, Idx + 1>();
        } else {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template begin<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal == 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t begin() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '}') {
            return 1;
        } else {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template begin<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Idx == sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        return -1;
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal > 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '}') {
            return GetFmtByIndex<FmtState::Normal, Cs...>::
                template end<Goal - 1, Idx + 1>();
        } else {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template end<Goal, Idx + 1>();
        }
    }
    template<size_t Goal, size_t Idx,
        std::enable_if_t<(Goal == 0 && Idx < sizeof...(Cs)), int> = 0>
    static constexpr ssize_t end() {
        constexpr Cts<Cs...> fmt;
        if(fmt[Idx] == '}') {
            return Idx - 1;
        } else {
            return GetFmtByIndex<FmtState::Brace, Cs...>::
                template end<Goal, Idx + 1>();
        }
    }
};

template<FmtState State, Char ...Cs>
struct CtsFmt;
template<Char ...Cs>
struct CtsFmt<FmtState::Normal, Cs...> {
    template<size_t Begin, size_t Idx, typename ...A,
        std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        return ss;
    }
    template<size_t Begin, size_t Idx, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && 
            Cts<Cs...>::template Get<Idx>() == '%'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        CtsFmt<FmtState::Percent, Cs...>::template fn<Begin, Idx + 1>(
            ss, std::forward<A>(a)...
        );
        return ss;
    }
    template<size_t Begin, size_t Idx, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && 
            Cts<Cs...>::template Get<Idx>() == '{'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        CtsFmt<FmtState::Brace, Cs...>::template fn<Idx, Idx + 1>(
            ss, std::forward<A>(a)...
        );
        return ss;
    }
    //不涉及特殊字符，直接放入流中
    template<size_t Begin, size_t Idx, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && 
            Cts<Cs...>::template Get<Idx>() != '%' &&
            Cts<Cs...>::template Get<Idx>() != '{'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        ss << Cts<Cs...>::template Get<Idx>();
        CtsFmt<FmtState::Normal, Cs...>::template fn<Begin, Idx + 1>(
            ss, std::forward<A>(a)...
        );
        return ss;
    }
};
template<Char ...Cs>
struct CtsFmt<FmtState::Percent, Cs...> {
    template<size_t Begin, size_t Idx, typename ...A,
        std::enable_if_t<(Idx == sizeof...(Cs)), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        return ss;
    }
    //%{或者%%，转义为{和%
    template<size_t Begin, size_t Idx, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && 
           (Cts<Cs...>::template Get<Idx>() == '{' ||
            Cts<Cs...>::template Get<Idx>() == '%')), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        ss << Cts<Cs...>::template Get<Idx>();
        CtsFmt<FmtState::Normal, Cs...>::
            template fn<Begin, Idx + 1>(ss, std::forward<A>(a)...);
        return ss;
    }
    //无需转义的字符，添加%和后面的一个字符
    template<size_t Begin, size_t Idx, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && 
            Cts<Cs...>::template Get<Idx>() != '%' &&
            Cts<Cs...>::template Get<Idx>() != '{'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A &&...a) {
        ss << '%' << Cts<Cs...>::template Get<Idx>();
        CtsFmt<FmtState::Normal, Cs...>::
            template fn<Begin, Idx + 1>(ss, std::forward<A>(a)...);
        return ss;
    }
};
template<Char ...Cs>
struct CtsFmt<FmtState::Brace, Cs...> {
    template<size_t Begin, size_t Idx, typename ...A,
        std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr std::stringstream &fn(
            std::stringstream &ss, 
            A &&...a) {
        return ss;
    }
    template<size_t Begin, size_t Idx, typename A0, 
        std::enable_if_t<(Idx < sizeof...(Cs) &&
            Cts<Cs...>::template Get<Idx>() == '}'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A0 a0) {
        constexpr Cts<Cs...> fmt;
        auto fmtFlag = FmtFlag::Get(fmt.template SubStr<Begin, Idx - Begin + 1>());
        ss << fmtFlag << a0;
        CtsFmt<FmtState::Normal, Cs...>::
            template fn<Begin, Idx + 1>(ss);
        return ss;
    }
    template<size_t Begin, size_t Idx, typename A0, 
        std::enable_if_t<(Idx < sizeof...(Cs) &&
            Cts<Cs...>::template Get<Idx>() != '}'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A0 a0) {
        CtsFmt<FmtState::Brace, Cs...>::template fn<Begin, Idx + 1>(
            ss, std::forward<A0>(a0)
        );
        return ss;
    }
    template<size_t Begin, size_t Idx, typename A0, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && sizeof...(A) > 0 &&
            Cts<Cs...>::template Get<Idx>() == '}'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A0 a0, A &&...a) {
        constexpr Cts<Cs...> fmt;
        auto fmtFlag = FmtFlag::Get(fmt.template SubStr<Begin, Idx - Begin + 1>());
        ss << fmtFlag << a0;
        CtsFmt<FmtState::Normal, Cs...>::
            template fn<Begin, Idx + 1>(ss, std::forward<A>(a)...);
        return ss;
    }
    template<size_t Begin, size_t Idx, typename A0, typename ...A, 
        std::enable_if_t<(Idx < sizeof...(Cs) && sizeof...(A) > 0 &&
            Cts<Cs...>::template Get<Idx>() != '}'), int> = 0>
    static constexpr std::stringstream &fn(std::stringstream &ss, A0 a0, A &&...a) {
        CtsFmt<FmtState::Brace, Cs...>::template fn<Begin, Idx + 1>(
            ss, std::forward<A0>(a0), std::forward<A>(a)...
        );
        return ss;
    }
};

}

template<Char ...Cs, typename ...A>
constexpr std::stringstream Fmtss(const Cts<Cs...> &fmt, A &&...a) {
    static_assert(__detail::GetFmtSize<__detail::FmtState::Normal, Cs...>::
        template fn<0>() <= sizeof...(a), "fmt size too much!");
    static_assert(__detail::GetFmtSize<__detail::FmtState::Normal, Cs...>::
        template fn<0>() >= sizeof...(a), "args size too much!");
    std::stringstream ret;
    __detail::CtsFmt<__detail::FmtState::Normal, Cs...>::
        template fn<0, 0>(ret, std::forward<A>(a)...);
    return ret;
}

template<Char ...Cs, typename ...A>
constexpr std::string Fmts(const Cts<Cs...> &fmt, A &&...a) {
    return Fmtss(fmt, std::forward<A>(a)...).str();
}

template<Char ...Cs, typename ...A>
constexpr size_t Fmt(const Cts<Cs...> &fmt, A &&...a) {
    auto &&str = Fmts(fmt, std::forward<A>(a)...);
    std::cout << str;
    return str.length();
}
}

#ifdef _FOT_TEST_
template<fmt::Char ...Cs>
constexpr size_t FmtSizeTest(const fmt::Cts<Cs...> &fmt) {
    return fmt::__detail::GetFmtSize<
        fmt::__detail::FmtState::Normal, Cs...
    >::template fn<0>();
}
template<size_t GoalIdx, fmt::Char ...Cs>
constexpr ssize_t FmtIdxBegin(const fmt::Cts<Cs...> &fmt) {
    return fmt::__detail::GetFmtByIndex<
        fmt::__detail::FmtState::Normal, Cs...
    >::template begin<GoalIdx, 0>();
}
template<size_t GoalIdx, fmt::Char ...Cs>
constexpr ssize_t FmtIdxEnd(const fmt::Cts<Cs...> &fmt) {
    return fmt::__detail::GetFmtByIndex<
        fmt::__detail::FmtState::Normal, Cs...
    >::template end<GoalIdx, 0>();
}

int main()
{
    DEF_CTS();
    constexpr auto flag = fmt::__detail::FmtFlag::Get(CTS("Hw32f0"));
    constexpr auto size = FmtSizeTest(CTS("{hw32f0}"));
    constexpr auto idx = FmtIdxBegin<0>(CTS("{hw32f0}"));
    constexpr auto idx2 = FmtIdxEnd<0>(CTS("{hw32f0}"));
    printf("flag: {%ld, %d, %d, %c}\n", 
        flag.width, flag.hex, flag.left, flag.fill);
    std::cout << "size: " << size << std::endl;
    std::cout << "idx: " << idx << std::endl;
    std::cout << "idx2: " << idx2 << std::endl;

    auto ss = fmt::Fmtss(CTS("%{0x{hw4f0}"), 23);
    std::cout << ss.str() << std::endl;
}
#endif



#endif
