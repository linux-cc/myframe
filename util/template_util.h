#ifndef __UTIL_TEMPLATE_UTIL_H__
#define __UTIL_TEMPLATE_UTIL_H__

#include <stdlib.h>

// Annotate a variable indicating it's ok if the variable is not used.
// (Typically used to silence a compiler warning when the assignment
// is important for some other reason.)
// Use like:   int x ALLOW_UNUSED = ...;
#ifdef __GNUC__
#define ALLOW_UNUSED __attribute__((unused))
#else
#define ALLOW_UNUSED
#endif /* ifdef __GNUC__ */

#define DISALLOW_COPY_AND_ASSIGN(Type) \
    Type(Type&);\
    Type& operator=(Type&)

#define DISALLOW_IMPLICIT_CONSTRUCTORS(Type) \
    Type();\
    DISALLOW_COPY_AND_ASSIGN(Type)

#if __cplusplus >= 201103L
// Under C++11, just use static_assert.
#define COMPILE_ASSERT(expr, msg) static_assert(expr, #msg)
#else
template <bool> struct CompileAssert {};
#define COMPILE_ASSERT(expr, msg) \
typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1] ALLOW_UNUSED
#endif /* #if __cplusplus >= 201103L */

namespace util {
    
typedef char YesType;
struct NoType {
    YesType dummy[2];
};

template<typename T, T v>
struct IntegralConstant {
    static const T value = v;
    typedef T value_type;
    typedef IntegralConstant<T, v> type;
};
template <typename T, T v> const T IntegralConstant<T, v>::value;

typedef IntegralConstant<bool, true> TrueType;
typedef IntegralConstant<bool, false> FalseType;

template <typename T> struct IsVoid: FalseType {};
template <> struct IsVoid<void>: TrueType {};

template <typename T> struct IsConst : FalseType {};
template <typename T> struct IsConst<const T> : TrueType {};

template<bool B, typename T = void> struct EnableIf {};
template<typename T> struct EnableIf<true, T> { typedef T type; };

template<typename> struct IsArray : FalseType {};
template<typename T, int n> struct IsArray<T[n]> : TrueType {};
template<typename T> struct IsArray<T[]> : TrueType {};

template <typename T> struct IsNonConstReference : FalseType {};
template <typename T> struct IsNonConstReference<T&> : TrueType {};
template <typename T> struct IsNonConstReference<const T&> : FalseType {};

template <typename T> struct IsPointer : FalseType {};
template <typename T> struct IsPointer<T*> : TrueType {};

template <typename T> struct IsTrivial : FalseType {};
template <typename T> struct IsTrivial<T*> : TrueType {};
template <> struct IsTrivial<bool> : TrueType {};
template <> struct IsTrivial<char> : TrueType {};
template <> struct IsTrivial<unsigned char> : TrueType {};
template <> struct IsTrivial<short> : TrueType {};
template <> struct IsTrivial<unsigned short> : TrueType {};
template <> struct IsTrivial<int> : TrueType {};
template <> struct IsTrivial<unsigned int> : TrueType {};
template <> struct IsTrivial<long> : TrueType {};
template <> struct IsTrivial<unsigned long> : TrueType {};
template <> struct IsTrivial<long long> : TrueType {};
template <> struct IsTrivial<unsigned long long> : TrueType {};
template <> struct IsTrivial<float> : TrueType {};
template <> struct IsTrivial<double> : TrueType {};
template <> struct IsTrivial<long double> : TrueType {};

template <typename T>
struct IsMoveOnlyType {
    template <typename U>
    static YesType test(const typename U::MoveOnlyTypeForCPP03*);

    template <typename U>
    static NoType test(...);

    static const bool value = sizeof(test<T>(NULL)) == sizeof(YesType) && !IsConst<T>::value;
};

struct ConvertHelper {
    template <typename To>
    static YesType test(To);

    template <typename To>
    static NoType test(...);

    template <typename From>
    static From& create();
};

template <typename From, typename To>
struct IsConvertible : IntegralConstant<bool,
    sizeof(ConvertHelper::test<To>(ConvertHelper::create<From>())) == sizeof(YesType)> {
};

}; /* namespace util */

#endif /* __UTIL_TEMPLATE_UTIL_H__ */
