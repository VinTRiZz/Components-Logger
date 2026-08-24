#pragma once

#include <type_traits>

#include <string>

namespace Logger
{

template <typename T, typename = void>
struct IsString : public std::false_type { };

template <typename T>
struct IsString<
    T,
    std::enable_if_t<
        std::disjunction_v<
        std::is_same<T, std::string>,
        std::is_same<T, std::string_view>
        > >
    > : public std::true_type { };


// ======================================================= //

template <typename T, typename = void>
struct IsContainer : public std::false_type { };

template <typename T>
struct IsContainer<
    T,
    std::void_t<
        decltype(T().begin()),
        decltype(T().end()),
        std::enable_if_t<!IsString<T>::value>
        > > : public std::true_type { };


// ======================================================= //

template <typename T, typename = void>
struct IsMap : public std::false_type { };

template <typename T>
struct IsMap<
    T,
    std::void_t<
        IsContainer<T>,
        typename T::key_type,
        typename T::value_type
        > > : public std::true_type { };

}
