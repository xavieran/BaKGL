#pragma once

#include <variant>

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, typename F, typename ...Ts>
decltype(auto) evaluate_if(std::variant<Ts...>& value, F&& function)
{
    using ReturnType = decltype(function(std::declval<T>()));
    if constexpr (!std::is_same_v<void, ReturnType>)
    {
        if (std::holds_alternative<T>(value))
            return function(std::get<T>(value));
        else
            return ReturnType{};
    }
    else
    {
        if (std::holds_alternative<T>(value))
            function(std::get<T>(value));
    }
}

template <typename T, typename F, typename ...Ts>
decltype(auto) evaluate_if(const std::variant<Ts...>& value, F&& function)
{
    using ReturnType = decltype(function(std::declval<T>()));
    if constexpr (!std::is_same_v<void, ReturnType>)
    {
        if (std::holds_alternative<T>(value))
            return function(std::get<T>(value));
        else
            return ReturnType{};
    }
    else
    {
        if (std::holds_alternative<T>(value))
            function(std::get<T>(value));
    }
}
