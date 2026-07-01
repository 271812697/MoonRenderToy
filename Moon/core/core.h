#pragma once
template <typename T>
constexpr T&& Move(T& value)
{
    return static_cast<T&&>(value);
}
template <typename T>
constexpr inline void Swap(T& t1, T& t2)
{
    T temp = Move(t1);
    t1 = Move(t2);
    t2 = Move(temp);
}

template <class T>
struct IsLValueReference
{
    static constexpr bool value = false;
};
template <class T>
struct IsLValueReference<T&>
{
    static constexpr bool value = true;
};
template <class T>
struct IsRValueReference
{
    static constexpr bool value = false;
};
template <class T>
struct IsRValueReference<T&&>
{
    static constexpr bool value = true;
};

/// RemovePointer removes the pointer qualification from a type `T`.
template <class T>
struct RemovePointer
{
    using type = T;
};
template <class T>
struct RemovePointer<T*>
{
    using type = T;
};
template <class T>
struct RemovePointer<T**>
{
    using type = T;
};
template <class T>
struct RemoveReference
{
    using type = T;
};
template <class T>
struct RemoveReference<T&>
{
    using type = T;
};
template <class T>
struct RemoveReference<T&&>
{
    using type = T;
};
template <class T>
struct AddPointer
{
    using type = typename RemoveReference<T>::type*;
};
template <bool B, class T = void>
struct EnableIf
{
};
template <class T>
struct EnableIf<true, T>
{
    using type = T;
};
template <typename T, typename U>
struct IsSame
{
    static constexpr bool value = false;
};
template <typename T>
struct IsSame<T, T>
{
    static constexpr bool value = true;
};

template <typename T>
constexpr T&& Forward(typename RemoveReference<T>::type& value)
{
    return static_cast<T&&>(value);
}

template <typename T>
constexpr T&& Forward(typename RemoveReference<T>::type&& value)
{
    static_assert(!IsLValueReference<T>::value, "Forward an rvalue as an lvalue is not allowed");
    return static_cast<T&&>(value);
}