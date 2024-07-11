#pragma once
#include <string_view>

#include "field_reflection.hpp"
#if __has_include(<concepts>) || defined(__clang__) || defined(_MSC_VER) || \
    (defined(__GNUC__) && __GNUC__ > 10)
#include "member_ptr.hpp"

namespace ylt::reflection {

namespace internal {

template <class T>
struct Wrapper {
  using Type = T;
  T v;
};

template <class T>
Wrapper(T) -> Wrapper<T>;

// This workaround is necessary for clang.
template <class T>
inline constexpr auto wrap(const T& arg) noexcept {
  return Wrapper{arg};
}

template <auto ptr>
inline constexpr std::string_view get_member_name() {
#if defined(_MSC_VER)
  constexpr std::string_view func_name = __FUNCSIG__;
#else
  constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif

#if defined(__clang__)
  auto split = func_name.substr(0, func_name.size() - 2);
  return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
  auto split = func_name.substr(0, func_name.size() - 2);
  return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
  auto split = func_name.substr(0, func_name.size() - 7);
  return split.substr(split.rfind("->") + 2);
#else
  static_assert(false,
                "You are using an unsupported compiler. Please use GCC, Clang "
                "or MSVC or switch to the rfl::Field-syntax.");
#endif
}
}  // namespace internal

template <typename T>
inline constexpr std::array<std::string_view, members_count_v<T>>
get_member_names() {
  constexpr size_t Count = members_count_v<T>;
  constexpr auto tp = struct_to_tuple<T>();

  std::array<std::string_view, Count> arr;
  [&]<size_t... Is>(std::index_sequence<Is...>) mutable {
    ((arr[Is] = internal::get_member_name<internal::wrap(std::get<Is>(tp))>()),
     ...);
  }
  (std::make_index_sequence<Count>{});
  return arr;
}

template <typename T>
inline constexpr auto get_member_names_map() {
  constexpr auto name_arr = get_member_names<T>();
  return [&]<size_t... Is>(std::index_sequence<Is...>) mutable {
    return frozen::unordered_map<frozen::string, size_t, name_arr.size()>{
        {name_arr[Is], Is}...};
  }
  (std::make_index_sequence<name_arr.size()>{});
}
}  // namespace ylt::reflection
#endif