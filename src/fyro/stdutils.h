#pragma once

#include <type_traits>


 // convert a enum class to it's underlying (int) type
// src: https://twitter.com/idoccor/status/1314664849276899328
template<typename E>
constexpr typename std::underlying_type<E>::type
base_cast(E e) noexcept
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}
