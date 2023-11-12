#ifndef _MY_TYPE_TRAITS_HPP_
#define _MY_TYPE_TRAITS_HPP_

#include<cstdlib> //size_t
#include<utility> //std::declval
#include<type_traits>

//std::xxx_t
template<typename T>
using decay_t = typename std::decay<T>::type;
template<bool flag, typename U = void>
using enable_if_t = typename std::enable_if<flag, U>::type;
// template<typename Callable, typename ...Args>
// using return_type = typename std::result_of<Callable(Args...)>::type;

/************* always false ***********/
template<typename ...A>
struct AlwaysFalse : std::false_type {};

/****************** pointer decay **************/
template<typename T>
struct ptr_decay { using Tp = T; };
template<typename T>
struct ptr_decay<T *> { using Tp = decay_t<T> *; };
template<typename T>
using ptr_decay_t = typename ptr_decay<decay_t<T>>::Tp;

/*********** is const ref or right ref *********/
template<typename T>
struct is_cref_or_rref : std::false_type {};
template<typename T>
struct is_cref_or_rref<const T &> : std::true_type {
    using CRef = const decay_t<T> &;
    using RRef = decay_t<T> &&;
    using Tp = T;
};
//**INDENT-OFF**
template<typename T>
struct is_cref_or_rref <T &&> : std::true_type {
    using CRef = const decay_t<T> &;
    using RRef = decay_t<T> &&;
    using Tp = T;
};
//**INDENT-ON**
template<typename T>
using is_crref = is_cref_or_rref<T>;
//T is Goal's const or right ref
template<typename T, typename Goal>
struct is_crref_of {
    static constexpr bool value = is_crref<T>::value && std::is_same<decay_t<T>, Goal>::value;
};

/***************** Num... traits ****************/
template<int index, int M, int ...N>
struct getNum: getNum < index - 1, N... > {};
template<int M, int ...N>
struct getNum<0, M, N...> { static constexpr int value = M; };

template<int ...N>
struct IntSeq: IntSeq < getNum<0, N...>::value - 1, N... > {};
template<int ...N>
struct IntSeq<0, N...> {
    using Tp = IntSeq<0, N...>;
};
template<>
struct IntSeq<> {};
template<>
struct IntSeq < -1 > { using Tp = IntSeq<>; };

template<int N>
using IntSeq_t = typename IntSeq < N - 1 >::Tp;
template<int N>
using MakeIntSeq = IntSeq_t<N>;

// IntSeq_t<0> s; // ==> IntSeq<>
// IntSeq_t<3> s; // ==> IntSeq<0, 1, 2>

/********** get member function qualifier *********/
template<typename MemFn>
struct get_mem_fn_qualifier {
    static_assert(AlwaysFalse<MemFn>::value, 
        "MemFn must be member function pointer!");
};
template<typename R, typename C, typename ...A>
struct get_mem_fn_qualifier<R(C::*)(A...)> {
    using type = C;
};
template<typename R, typename C, typename ...A>
struct get_mem_fn_qualifier<R(C::*)(A...) const> {
    using type = const C;
};
template<typename R, typename C, typename ...A>
struct get_mem_fn_qualifier<R(C::*)(A...) volatile> {
    using type = volatile C;
};
template<typename R, typename C, typename ...A>
struct get_mem_fn_qualifier<R(C::*)(A...) const volatile> {
    using type = const volatile C;
};
template<typename T>
using get_mem_fn_qualifier_t = typename get_mem_fn_qualifier<T>::type;


/******** remove member function qualifier ********/
template<typename Callable>
struct mem_fn_remove_const {
    // static_assert(std::is_function<Callable>::value
    //               || std::is_member_function_pointer<Callable>::value,
    //               "Callable is not function");
    using Tp = Callable;
};
template<typename R, typename C, typename ...A>
struct mem_fn_remove_const<R(C::*)(A...) const> {
    using Tp = R(C::*)(A...);
};
template<typename R, typename C, typename ...A>
struct mem_fn_remove_const<R(C::*)(A...) const volatile> {
    using Tp = R(C::*)(A...) volatile;
};

template<typename Callable>
using mem_fn_remove_const_t = typename mem_fn_remove_const<Callable>::Tp;

template<typename Callable>
struct mem_fn_remove_volatile {
    // static_assert(std::is_function<Callable>::value
    //               || std::is_member_function_pointer<Callable>::value,
    //               "Callable is not function");
    using Tp = Callable;
};
template<typename R, typename C, typename ...A>
struct mem_fn_remove_volatile<R(C::*)(A...) volatile> {
    using Tp = R(C::*)(A...);
};
template<typename R, typename C, typename ...A>
struct mem_fn_remove_volatile<R(C::*)(A...) const volatile> {
    using Tp = R(C::*)(A...) const;
};
template<typename Callable>
using mem_fn_remove_volatile_t = typename mem_fn_remove_volatile<Callable>::Tp;

template<typename Callable>
using mem_fn_remove_cv_t = mem_fn_remove_const_t<mem_fn_remove_volatile_t<Callable>>;
template<typename Callable>
using mem_fn_decay_t = typename std::remove_reference<mem_fn_remove_cv_t<Callable>>::type;

/******** function's return value type ********/
#if 0
template<typename Callable, typename ...Args>
struct ret_type_of {
    using Tp = decltype(std::declval<Callable>()(std::declval<Args>()...));
};
template<typename Callable, typename T, typename ...Args>
struct ret_type_of<Callable T::*, Args...> {
    using Tp = decltype((std::declval<T>().*std::declval<Callable T::*>())(std::declval<Args>()...));
};
#else
template<typename F>
struct ret_type_of;
template<typename R, typename ...A>
struct ret_type_of<R(*)(A...)> {
    using Tp = R;
};
template<typename R, typename C, typename ...A>
struct ret_type_of<R(C::*)(A...)> {
    using Tp = R;
};
template<typename C>
struct ret_type_of {
    using Tp = typename ret_type_of<mem_fn_decay_t<decltype(&C::operator())>>::Tp;
};
#endif
template<typename Callable>
using ret_type_t = typename ret_type_of<mem_fn_decay_t<decay_t<Callable>>>::Tp;

/*************** get template list's type ****************/
template<typename Arg1, typename ...Args>
struct getType0_ { using Tp = Arg1; };
template<typename ...Args>
struct getType0 { using Tp = typename getType0_<Args...>::Tp; };
template<typename ...Args>
using getType0_t = typename getType0<Args...>::Tp;

template<size_t N, typename Arg0, typename ...Args>
struct getTypeN: getTypeN < N - 1, Args... >  {};
template<typename Arg0, typename ...Args>
struct getTypeN<0, Arg0, Args...> { using Tp = Arg0; };
template<size_t N, typename ...Args>
using getTypeN_t = typename getTypeN<N, Args...>::Tp;

//**INDENT-OFF**
template<int ...N>
struct RangeSeq {};
template<int Begin, int ...N>
constexpr auto MakeRangeSeqHelper(IntSeq<N...> s) -> RangeSeq<(Begin + N)...> { return {}; }
template<int Begin, int Num>
constexpr auto MakeRangeSeq() -> decltype(MakeRangeSeqHelper<Begin>(IntSeq_t<Num> {}))
{ return MakeRangeSeqHelper<Begin>(IntSeq_t<Num> {}); }
//constexpr auto seq = MakeRangeSeq<5, 3>(); // ==> RangeSeq<5, 6, 7>;
//**INDENT-ON**

/*******get template list's element*******/
template<typename Type, Type Element, Type ...Elements>
struct getEle0_ { static constexpr Type value = Element; };
template<typename Type, Type ...Elements>
struct getEle0 { static constexpr Type value = getEle0_<Type, Elements...>::value; };

template<size_t N, typename Type, Type Element, Type ...Elements>
struct getEleN_: getEleN_ < N - 1, Type, Elements... > {};
template<typename Type, Type Element, Type ...Elements>
struct getEleN_<0, Type, Element, Elements...> {
    static constexpr Type value = Element;
};
template<size_t N, typename Type, Type ...Elements>
struct getEleN { static constexpr Type value = getEleN_<N, Type, Elements...>::value; };

/************** get function template params ***********/
template<typename A0, typename ...A>
A0 GetArgs0(A0 &&a0, A &&...a) { return a0; }
template<size_t idx, typename ...A>
getType0_t<A...> GetArgs(A &&...a) { return GetArgs0(std::forward<A>(a)...); }

/*******get functor param list's type*******/
// 判断是否是仿函数类的类型特征类
template <typename T>
struct is_functor {
    template <typename C>
    static std::true_type test(decltype(&C::operator()));
    template <typename C>
    static std::false_type test(...);
    static constexpr bool value = decltype(test<decay_t<T>>(nullptr))::value;
};
// class Type { void f() {}; void operator()() const {} };
// constexpr auto b0 = is_functor<Type>::value; //false: Type::operator() is private

/********** is normal function pointer *******/
//true if is function type of function pointer
template<typename T>
struct is_normal_fn_ptr : std::false_type {};
template<typename R, typename ...Args>
struct is_normal_fn_ptr<R(*)(Args...)> : std::true_type {};
template<typename R, typename ...Args>
struct is_normal_fn_ptr<R(Args...)> : std::true_type {};

/********* get function param list package ******/
template<typename ...Args>
struct ArgsPkg {
    static constexpr size_t value = sizeof...(Args);
};
template <typename Callable>
struct ArgsPkgExtractor;
template <typename Ret, typename... Args>
struct ArgsPkgExtractor<Ret(Args...)> {
    using Tp = ArgsPkg<Args...>;
};
template <typename Ret, typename... Args>
struct ArgsPkgExtractor<Ret(*)(Args...)> {
    using Tp = ArgsPkg<Args...>;
};
template <typename Ret, typename Functor, typename... Args>
struct ArgsPkgExtractor<Ret(Functor::*)(Args...)> {
    using Tp = ArgsPkg<Args...>;
};
template <typename Callable>
struct ArgsPkgExtractor {
    // using Tp = typename ArgsPkgExtractor<mem_fn_decay_t<decltype(&Callable::operator())>>::Tp;
    using Tp = typename ArgsPkgExtractor<mem_fn_decay_t<decltype(&Callable::operator())>>::Tp;
};
template <typename Callable>
using ArgsPkg_t = typename ArgsPkgExtractor<mem_fn_decay_t<decay_t<Callable>>>::Tp;


/********** enable if T is/not member funtion pointer ********/
template<typename T, typename U = void>
using enable_if_is_mem_fn_ptr = enable_if_t<std::is_member_function_pointer<T>::value, U>;
template<typename T, typename U = void>
using enable_if_not_mem_fn_ptr = enable_if_t < !std::is_member_function_pointer<T>::value, U >;

/************** class of the member funtion ****************/
template<typename T>
struct class_of {};
template<typename R, typename C, typename ...A>
struct class_of<R(C::*)(A...)> { using Tp = C; };
template<typename R, typename C, typename ...A>
struct class_of<R(C::*)(A...) const> { using Tp = const C; };
template<typename R, typename C, typename ...A>
struct class_of<R(C::*)(A...) volatile> { using Tp = volatile C; };
template<typename R, typename C, typename ...A>
struct class_of<R(C::*)(A...) const volatile> { using Tp = const volatile C; };
template<typename Callable>
using class_of_t = typename class_of<mem_fn_decay_t<decay_t<Callable>>>::Tp;
// struct Type1 {
//     void f(int);
// };
// constexpr auto b = std::is_same<class_of_t<decltype(&Type1::f)>, Type1>::value;  //true

/********* functor judge *******/
template<typename Goal>
struct is_functor_type;
template<typename R, typename ...Args>
struct is_functor_type<R(Args...)> {
    //**INDENT-OFF**
    template<typename T,
             enable_if_t<!is_functor<T>::value
                && !is_normal_fn_ptr<T>::value
                && !std::is_member_function_pointer<T>::value>* = nullptr>
    static constexpr bool As(...) { return false; }
    //**INDENT-ON**
    //for functor object as function param
    template<typename T,
             enable_if_t<is_functor<T>::value>* = nullptr>
    static constexpr bool As(T && t)
    {
        using OpTp = mem_fn_decay_t<decltype(&decay_t<T>::operator())>;
        return std::is_same<OpTp, R(decay_t<T>::*)(Args...)>::value;
    }
    //**INDENT-OFF**
    template<typename T,
             enable_if_t<is_normal_fn_ptr<T>::value
                && std::is_same<decay_t<T>, R(Args...)>::value>* = nullptr>
    static constexpr bool As(T &&t) { return true; }
    template<typename T,
             enable_if_t<is_normal_fn_ptr<T>::value
                && std::is_same<decay_t<T>, R(*)(Args...)>::value>* = nullptr>
    static constexpr bool As(T &&t) { return true; }
    template<typename T, 
             enable_if_t<std::is_member_function_pointer<T>::value
                && std::is_same<mem_fn_decay_t<decay_t<T>>, R(class_of_t<T>::*)(Args...)>::value>* = nullptr>
    static constexpr bool As(T &&t) { return true; }
    //**INDENT-ON**
};
// auto llll = [](){};
// void lll() {}
// constexpr auto bb = is_functor<decltype(llll)&>::value;
// constexpr auto bbbb = is_functor_type<void(int)>::As(llll);

template<typename T, typename ...Args>
struct is_functor_args;
template<typename R, typename T, typename ...Args>
struct is_functor_args<R(T::*)(Args...), Args...> : std::true_type {};
template<typename R, typename ...Args>
struct is_functor_args<R(*)(Args...), Args...> : std::true_type {};
template<typename R, typename ...Args>
struct is_functor_args<R(Args...), Args...> : std::true_type {};
template<typename ...Args>
struct is_functor_args<std::false_type, Args...> : std::false_type {};
template<typename T, typename ...Args>
struct is_functor_args {
    template<typename U>
    static std::false_type test(...);
    template<typename U>
    static auto test(decltype(&U::operator())) -> decltype(&U::operator());
    using TestTp = decltype(test<T>(nullptr));
    static constexpr bool value = is_functor_args<mem_fn_decay_t<TestTp>, Args...>::value;
};
// void ff(int) {};
// void fff() {};
// constexpr auto b = is_functor_args<decltype(ff), int>::value; //true
// constexpr auto bb = is_functor_args<decltype(ff), double>::value; //false
// constexpr auto bbb = is_functor_args<decltype(fff), double>::value; //false
// constexpr auto bbbb = is_functor_args<decltype(fff)>::value; //true

template<typename T, typename R>
struct is_functor_ret;
template<typename R, typename T, typename ...Args>
struct is_functor_ret<R(T::*)(Args...), R> : std::true_type {};
template<typename R, typename ...Args>
struct is_functor_ret<R(Args...), R> : std::true_type {};
template<typename R, typename ...Args>
struct is_functor_ret<R(*)(Args...), R> : std::true_type {};
template<typename R>
struct is_functor_ret<std::false_type, R> : std::false_type {};
template<typename T, typename R>
struct is_functor_ret {
    template<typename U>
    static std::false_type test(...);
    template<typename U>
    static auto test(decltype(&U::operator())) -> decltype(&U::operator());
    using TestTp = decltype(test<T>(nullptr));
    static constexpr bool value = is_functor_ret<mem_fn_decay_t<TestTp>, R>::value;
};
// void ff(int) {};
// void fff() {};
// double ffff(float) {};
// constexpr auto b = is_functor_ret<decltype(ff), void>::value; //true
// constexpr auto bb = is_functor_ret<decltype(fff), double>::value; //false
// constexpr auto bbb = is_functor_ret<decltype(ffff), double>::value; //true
// constexpr auto bbbb = is_functor_ret<decltype(fff)>::value; //error

/********* overload of functions ********/
template<typename RetTp>
constexpr auto overload_of(RetTp(*fn)()) -> decltype(fn) { return fn; }
template<typename ...Args, typename RetTp>
constexpr auto overload_of(RetTp(*fn)(Args...)) -> decltype(fn) { return fn; }
template<typename T, typename RetTp>
constexpr auto overload_of(RetTp(T::*fn)()) -> decltype(fn) { return fn; }
template<typename ...Args, typename T, typename RetTp>
constexpr auto overload_of(RetTp(T::*fn)(Args...)) -> decltype(fn) { return fn; }
template<typename ...Args, typename T, typename RetTp>
constexpr auto overload_of_c(RetTp(T::*fn)(Args...) const) -> decltype(fn) { return fn; }
template<typename ...Args, typename T, typename RetTp>
constexpr auto overload_of_v(RetTp(T::*fn)(Args...) volatile) -> decltype(fn) { return fn; }
template<typename ...Args, typename T, typename RetTp>
constexpr auto overload_of_cv(RetTp(T::*fn)(Args...) const volatile) -> decltype(fn) { return fn; }
// void fn(int);
// void fn(double);
// int fn(float);
// constexpr auto ptr1 = overload_of<double>(fn);   //void(double)
// constexpr auto ptr2 = overload_of<float>(fn);   //int(float)

/*********** is type callable **********/
template<typename T>
struct is_callable_ : std::false_type {};
template<typename R, typename ...A>
struct is_callable_<R(A...)> : std::true_type {};
template<typename R, typename ...A>
struct is_callable_<R(*)(A...)> : std::true_type {};
template<typename R, typename C, typename ...A>
struct is_callable_<R(C::*)(A...)> : std::true_type {};
template<typename T>
struct is_callable {
    static constexpr bool value = is_functor<decay_t<T>>::value ||
        is_callable_<decay_t<mem_fn_decay_t<T>>>::value;
};

// class Type { void f() {}; void operator()() const {} };
// constexpr auto b0 = is_callable<int()>::value;//true
// constexpr auto b1 = is_callable<int(*)()>::value;//true
// constexpr auto b2 = is_callable<void(Type::*)() const>::value;//true
// constexpr auto b3 = is_callable<Type>::value;//==> false: operator() is private
// constexpr auto b4 = is_callable<int>::value;//false

#endif
