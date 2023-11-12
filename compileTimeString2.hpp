#ifndef _COMPILE_TIME_STRING_2_HPP__
#define _COMPILE_TIME_STRING_2_HPP__

#include<iostream>
#include<sstream>
#include<cstring>

#include "myTypeTraits.hpp"

#if __cplusplus < 201400
#error "c++ standard must >= c++14/gnu++14"
#endif

namespace cts
{
using Char = char;
template<Char ...Cs>
struct Str;

//**INDENT-OFF**
namespace __detail {
//打印变长字符模板参数
template<Char c>
constexpr std::ostream &print(std::ostream &o) {
    return o << c;
}
template<Char c, Char ...cs, 
    enable_if_t<(sizeof...(cs) > 0), int> = 0>
constexpr std::ostream &print(std::ostream &o) {
    // static_assert(sizeof...(cs) > 0, "");
    o << c;
    return print<cs...>(o);
}
template<Char c>
constexpr std::stringstream &sstream(std::stringstream &ss) {
    ss << c;
    return ss;
}
template<Char c, Char ...cs, 
    enable_if_t<(sizeof...(cs) > 0), int> = 0>
constexpr std::stringstream &sstream(std::stringstream &ss) {
    // static_assert(sizeof...(cs) > 0, "");
    ss << c;
    return sstream<cs...>(ss);
}

//获取子串，只需要一个函数声明即可。重要的是返回值类型
template<Char ...Cs, int ...Idx>
auto GetSubStr(RangeSeq<Idx...>) -> Str<getEleN<Idx, Char, Cs...>::value...>;

//字符串匹配
template<Char ...From, Char ...Goal, int ...N,
    enable_if_t<(sizeof...(From) < sizeof...(Goal)), int> = 0>
constexpr ssize_t Find(const Str<From...> &s1, 
                       const Str<Goal...> &s2, 
                       const IntSeq<N...> &_intseq
) {  //From... 还没有Goal.. 长，自然就是找不到，返回-1
    // static_assert(sizeof...(From) < sizeof...(Goal), "");
    return -1;
}
template<Char From1, Char ...From, Char ...Goal, int ...N,
    enable_if_t<(sizeof...(From) + 1 >= sizeof...(Goal)), int> = 0>
constexpr ssize_t Find(const Str<From1, From...> &s1, 
                       const Str<Goal...> &s2, 
                       const IntSeq<N...> &_intseq
) {
    // static_assert(sizeof...(From) + 1 >= sizeof...(Goal), "");
    using FromStr = Str<getEleN<N, Char, From1, From...>::value...>;
    if(std::is_same<FromStr, Str<Goal...>>::value) {
        //如果前N...个匹配了，返回0
        return 0;
    } else if(Find(Str<From...>{}, s2, _intseq) >= 0) {
        //递归查找，找到0了，就代表找到了，结果+1返回给上一层
        //从而可以在出递归的时候返回正确位置
        return 1 + Find(Str<From...>{}, s2, _intseq);
    } else {
        //找不到，直接返回-1
        return -1;
    }
}

//constexpr的strncpy，用于后面的NStr的constexpr初始化构造
template<size_t Idx>
constexpr void Strncpy(char *dst, const char *src) {
    dst[Idx] = src[Idx];
    Strncpy<Idx - 1>(dst, src);
}
template<>
constexpr void Strncpy<0>(char *dst, const char *src) {
    dst[0] = src[0];
}
template<size_t Idx>
constexpr size_t Strlen(const char *s) {
    if(s[Idx] != '\0') {
        return Idx + 1;
    } else {
        return Strlen<Idx - 1>(s);
    }
}
template<>
constexpr size_t Strlen<0>(const char *s) {
    return s[0] != '\0';
}
//constexpr变长字符串
template<size_t N>
struct NStr {
    template<size_t Len>
    static constexpr NStr Get(const char *s) {
        NStr ret;
        Strncpy<Len - 1>(ret.str, s);
        return ret;
    }
    constexpr size_t Length() const { return Strlen<N - 1>(str); }
    char str[N] = "";
    static constexpr size_t Num = N;
};
template<size_t N, const NStr<N> &ns, int ...M>
constexpr auto GetCts(IntSeq<M...>) -> Str<ns.str[M]...> { return {}; }

}
//**INDENT-ON**

template<Char ...Cs>
struct Str {
    static constexpr size_t N = sizeof...(Cs) + 1;
    static constexpr const Char arry[N] = {Cs..., '\0'};

    //operator<<
    friend std::ostream &operator<<(std::ostream &o, const Str &s) 
    { return __detail::print<Cs...>(o); }
    // {return o << Str::arry; }
    friend std::stringstream &operator<<(std::stringstream &ss, const Str &s) 
    { return __detail::sstream<Cs...>(ss); }
    // { ss << Str::arry; return ss; }
    //operator[]
    constexpr Char operator[](size_t n) const { return n >= N ? '\0' : arry[n]; }
    //operator==
    template<Char ...Cs2>
    constexpr bool operator==(const Str<Cs2...> &s2) const
    { return std::is_same<Str<Cs...>, Str<Cs2...>>::value; }
    template<Char ...Cs2>
    constexpr bool operator!=(const Str<Cs2...> &s2) const
    { return !std::is_same<Str<Cs...>, Str<Cs2...>>::value; }

    static constexpr size_t Len() { return N - 1; }
    
    //按index获取字符
    template<size_t Idx, std::enable_if_t<(Idx < sizeof...(Cs)), int> = 0>
    static constexpr Char Get() { return getEleN<Idx, Char, Cs...>::value; }
    template<size_t Idx, std::enable_if_t<(Idx >= sizeof...(Cs)), int> = 0>
    static constexpr Char Get() { return '\0'; }

    //获取子串
    template<int Begin, int Num>
    static constexpr auto SubStr()
    { return decltype(__detail::GetSubStr<Cs...>(MakeRangeSeq<Begin, Num>())){}; };

    //反向获取子串
    template<int Num>
    static constexpr auto RSubStrN()
    { return decltype(__detail::GetSubStr<Cs...>(MakeRangeSeq<Len() - Num, Num>())){}; }
    template<int Idx>
    static constexpr auto RSubStr()
    { return decltype(__detail::GetSubStr<Cs...>(MakeRangeSeq<Idx, Len() - Idx>())){}; }

    //字符串匹配
    template<Char ...Goal>
    static constexpr ssize_t Find(Str<Goal...> s) {
        return __detail::Find(Str{}, s, IntSeq_t<sizeof...(Goal)>{});
    }
    
    //判断字符集合包含
    template<Char C, Char ...Cs2, 
        std::enable_if_t<(sizeof...(Cs2) > 0), int> = 0>
    static constexpr bool Contains() {
        return Find(Str<C>{}) >= 0 || Contains<Cs2...>();
    }
    template<Char C, Char ...Cs2, 
        std::enable_if_t<(sizeof...(Cs2) == 0), int> = 0>
    static constexpr bool Contains()  {
        return Find(Str<C>{}) >= 0;
    }

    //字符串拼接
    template<Char ...Cs2>
    constexpr Str<Cs..., Cs2...> operator+(const Str<Cs2...> &s) { return {}; }
};
//static constexpr变量在c++14及之前，都需要类外初始化一下
template<Char ...Cs>
constexpr const Char Str<Cs...>::arry[N];

//字符串定义、初始化接口
template <typename T, T ...Chars>
constexpr auto operator""_cts()
{
    return Str<Chars...> {};
}
//编译期字符串获取的宏函数，先用DEF_CTS来声明，然后可以使用CTS
#define DEF_CTS() using cts::operator""_cts;
#define CTS(str) str""_cts

namespace tool {
/******************
 * 
 *          编译期获取T的类型名，并提供打印（std::cout），要求c++14及以上
 * 
 * Usage: std::cout << cts::tool::GetTypeCts<MyType>() << std::endl;
 *   ==> MyType
 *******************/
template<typename T>
constexpr auto GetTypeCtsHelper() {
    constexpr size_t size = sizeof(__PRETTY_FUNCTION__);
    return __detail::NStr<128>::template Get<size>(__PRETTY_FUNCTION__);
}
template<typename T>
constexpr __detail::NStr<128> nStr = GetTypeCtsHelper<T>();
template<typename T>
constexpr size_t nStrNum = nStr<T>.Length();
template<typename T>
auto GetTypeCts() {
    // static constexpr auto nStr = GetTypeCtsHelper<T>();
    constexpr size_t prefixSize = 
        sizeof("constexpr auto cts::tool::GetTypeCtsHelper() [with T = ") - 1;
    constexpr size_t typeSize = nStrNum<T> - (sizeof("]") - 1) - prefixSize;
    constexpr auto tmpStr = __detail::GetCts<128, nStr<T>>(IntSeq_t<nStrNum<T>>{});
    return tmpStr.template SubStr<prefixSize, typeSize>();
    // return tmpStr;
}
// 字符串测试：
// constexpr auto nStrNum = cts::tool::nStrNum<double>;
// constexpr auto nStrNum2 = cts::tool::nStr<double>.Length();
// "constexpr auto cts::tool::GetTypeCtsHelper() [with T = double]";

/******************
 * 
 *          打印T的类型名，c++版本要求同GetTypeCts
 *  Usage: CTS_TOOL_PrintType(Type)
 *  template<typename T>
 *  void f() {
 *      CTS_TOOL_PrintType(T);
 *  }
 * 
 *******************/
template<typename T>
constexpr void PrintType(const char *file, size_t line, const char *prefix)
{
    std::cout << "[" << file << " :" << line
              << "]: " << prefix << " = "
              << cts::tool::GetTypeCts<T>() << std::endl;
}
#define CTS_TOOL_PrintType(X) cts::tool::PrintType<X>(__FILE__, __LINE__, #X)

/******************
 * 
 *          编译期从宏获取文件名（__FILE__）
 *  Usage: constexpr auto filename = Filename(CTS(__FILE__))
 *  Usage: 从宏：CTS_TOOL_FILE
 * 
 *******************/
template<size_t Idx>
struct FilenameHelper;
template<>
struct FilenameHelper<0> {
    template<Char ...Cs>
    static constexpr auto fn(const Str<Cs...> &none) {
        return none;
    }
};
template<size_t Idx>
struct FilenameHelper {
    template<Char ...Cs, 
        std::enable_if_t<
            getEleN<Idx, Char, Cs...>::value == '\\'
          ||getEleN<Idx, Char, Cs...>::value == '/', int> = 0
    >
    static constexpr auto fn(const Str<Cs...> &fullpath)
    {
        return fullpath.template RSubStr<Idx + 1>();
    }
    template<Char ...Cs, 
        std::enable_if_t<
            getEleN<Idx, Char, Cs...>::value != '\\'
          &&getEleN<Idx, Char, Cs...>::value != '/', int> = 0
    >
    static constexpr auto fn(const Str<Cs...> &fullpath)
    {
        return FilenameHelper<Idx - 1>::fn(fullpath);
    }
};
template<Char ...FileCs>
constexpr auto Filename(const Str<FileCs...> &fileCts) {
    return FilenameHelper<fileCts.Len() - 1>::fn(fileCts);
}
#define CTS_TOOL_FILE cts::tool::Filename(CTS(__FILE__))
}

}

#endif
