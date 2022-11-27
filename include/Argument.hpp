/*****************************
 * lfl/Argument
 *****************************/

#pragma once

#include <string>
#include <cstring>
#include <utility>

#ifndef OPTION_LIST
#define OPTION_LIST { "directory", "help" }
#endif

namespace ARG {

using string_type = char const *;
using list_type = string_type [];
using index_type = std::size_t;
using index_type_const = index_type const;
using length_type = std::size_t;

static constexpr list_type defined_list = OPTION_LIST;

template<typename> constexpr bool empty_list = false;

template<class T = void>
static consteval std::size_t ArraySize() {
  if constexpr ( std::extent_v<decltype( defined_list )> != 0 ) {
    return std::extent_v<decltype( defined_list ), 0>;
  } else {
    static_assert( empty_list<T>, "OPTION_LIST is empty");
    return std::extent_v<decltype( defined_list ), 0>;
  }
}

namespace OPTION {

namespace STRING {

template<index_type> constexpr bool wrong_id = false;

template<index_type ID>
static consteval string_type HeadAddress() {
  if constexpr ( ID < ArraySize() ) {
    return defined_list[ID];
  } else {
    static_assert( wrong_id<ID> , "specified ID is grater than number of option"); return nullptr;
  }
}

template<index_type ID, index_type INDEX = 0, char C = HeadAddress<ID>()[INDEX]>
struct Length {
  static constexpr length_type value = Length<ID, INDEX + 1>::value;
};

template<index_type ID, index_type INDEX>
struct Length<ID, INDEX, '\0'> {
  static constexpr length_type value = INDEX;
};

} // STRING

// template<index_type OPTION_NUM = ArraySize()>
// class LengthList;

class List;

// オプションの長さを実装するクラス
template<length_type OPTION_NUM = ArraySize()>
struct LengthListImpl {
  friend class List;
public:
  decltype(STRING::Length<0>::value) length[OPTION_NUM];

private:
  template<index_type... index_list>
  constexpr LengthListImpl( std::index_sequence<index_list...> ) : length{ STRING::Length<index_list>::value ... } {}
};

class List {
private:
  // constexpr char const * option_specificator = { "--" };

  static constexpr list_type option_list = OPTION_LIST;
  static constexpr decltype( std::extent<decltype( option_list ), 0>::value ) option_num = ArraySize();
  static constexpr LengthListImpl<> length = LengthListImpl<>( std::make_index_sequence<option_num>() ); // std::make_index_sequence<>はC++14から

  length_type countLength();
public:
  List() = default;
  ~List() = default;
  List( List const & ) = delete;
  List & operator=( List const & ) = delete;
  List( List && ) = delete;
  List & operator=( List && ) = delete;

  index_type Discriminate( string_type arg_option );
  index_type getNum() const noexcept { return option_num; }

  decltype( option_num ) getString( index_type );
};

} // OPTION

} // ARG
