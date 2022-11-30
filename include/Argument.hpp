/*****************************
 * lfl/Argument
 *****************************/

#pragma once

#include <string>
#include <cstring>
#include <utility>
#include <type_traits>

namespace UTIL {
  template<bool Cond> using if_nullp_c = std::enable_if_t<Cond, std::nullptr_t>;
} // UTIL

namespace ARG {

using string_type = char const *;
using list_type = string_type [];
using index_type = std::size_t;
using size_type = std::size_t;

template<typename T> constexpr bool false_v = false;

// Array_pが配列型でなければエラーとする。
// std::extent_vは、与えられた型が配列型でなければ0をvalueの値として定義するが、
// この関数は配列型かどうかを調べるために使いたいわけではない。
// だからここでは、配列型以外が与えられるとエラーを出すようにしている。
template<typename Array_p, index_type INDEX = 0, UTIL::if_nullp_c<std::is_array_v<Array_p>>* = nullptr>
consteval std::size_t ArraySize( Array_p const & p ) { return std::extent_v<Array_p, INDEX>; }

template<typename Array_p, index_type INDEX = 0, UTIL::if_nullp_c<!( std::is_array_v<Array_p> )>* = nullptr>
consteval std::size_t ArraySize( Array_p const & p ) { static_assert( false_v<Array_p>, "given type is NOT a pointer to array type"); return 0; }

namespace OPTION {

namespace STRING {

consteval auto Length( char const * const string ) {
  index_type index = 0;
  while ( string[index] != '\0' ) { ++index; }
  return index;
}

} // STRING

class List;

// オプションの長さを実装するクラス
// ただし、Tpは配列型でなければならないという制限を付けたい。
struct LengthListImpl {
  friend class List;
public:
  size_type value[];

private:
  template<typename T, index_type... index_list, UTIL::if_nullp_c<std::is_pointer_v<T>>* = nullptr>
  consteval LengthListImpl( T const string_list, std::index_sequence<index_list...> ) : value{ STRING::Length(string_list[index_list]) ... } {}
};

struct StringImpl {
  friend class List;
public:
  string_type value[];

private:
  template<typename T, index_type... index_list, UTIL::if_nullp_c<std::is_pointer_v<T>>* = nullptr>
  consteval StringImpl( T const string_list, std::index_sequence<index_list...> ) : value{ string_list[index_list] ... } {}
};

class List {
private:
  size_type option_num_;
  LengthListImpl length_list_;
  // StringImpl option_string_list_;

public:
  template<typename Tp, std::size_t OPTION_NUM = std::extent_v<Tp>, UTIL::if_nullp_c<
    std::is_array_v<Tp> && std::is_pointer_v<std::remove_extent_t<Tp>>
  >* = nullptr>
  consteval List( Tp const & option_list )
  : option_num_( OPTION_NUM )
  , length_list_( LengthListImpl( option_list, std::make_index_sequence<OPTION_NUM>() ) ) {} // std::make_index_sequence<>はC++14から
  // , option_list_( OptionStringImpl( option_list, std::make_index_sequence<OPTION_NUM>() ) ) {}

  // ~List() = default;
  // List( List const & ) = delete;
  // List & operator=( List const & ) = delete;
  // List( List && ) = delete;
  // List & operator=( List && ) = delete;

  constexpr auto num() const noexcept { return option_num_; }
  constexpr auto getLength( index_type const index ) const noexcept { return length_list_.value[index]; }

  // index_type Discriminate( string_type arg_option );
  // index_type getNum() const noexcept { return option_num_; }

  // decltype( option_num_ ) getString( index_type );
};

} // OPTION

} // ARG
