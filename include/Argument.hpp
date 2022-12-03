/*****************************
 * lfl/Argument
 *****************************/

#pragma once

#include <string>
#include <cstring>
#include <utility>
#include <type_traits>
#include <algorithm>

#define STATIC_CONSTEXPR static constexpr

namespace UTIL {
  // template<typename T> constexpr bool false_v = false;

  template<bool Cond> using if_nullp_c = std::enable_if_t<Cond, std::nullptr_t>;
} // UTIL

namespace ARG {

using char_cptr = char const *;
using list_type = char_cptr [];
using index_type = std::size_t;
using size_type = std::size_t;

template <typename T, size_type SIZE>
constexpr size_type ArraySize( T (&)[SIZE] ) { return SIZE; }

namespace OPTION {

namespace STRING {

consteval index_type Length( char const * const string ) {
  index_type index = 0;
  while ( string[index] != '\0' ) { ++index; }
  return index;
}

// N文字だけ比較して、同一文字列だったらtrue、
// 同一文字列でなければfalseを返す。
// ただし、str1とstr2が同じであっても、
// 末尾に'\0'を含んでいたらfalseを返してしまう(仕様と書いてルビはバグ)。
template<typename T, UTIL::if_nullp_c<std::is_pointer_v<T>>* = nullptr>
constexpr bool IsSameN( T const str1, T const str2, size_type const N ) {
  index_type index = 0;

  while ( index < N ) {
    if ( str1[index] != str2[index] || str1[index] == '\0' || str2[index] == '\0' ) { return false; }

    ++index;
  }

  return true;
}

// 最後の'\0'は含めたくない。
// というよりも、コンパイル時に要素数も分かるから必要ない。
template<typename CharT, size_type N>
struct StringLiteral {
  consteval StringLiteral( CharT const ( & string_literal )[N + 1] ) {
    std::copy_n( string_literal, N, str_chars_ );
  }

  CharT str_chars_[N];

  constexpr std::pair<CharT const *, decltype( N )> get() const noexcept { return { str_chars_, N }; }
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
StringLiteral( CharT const ( & literal )[N] ) -> StringLiteral<CharT, N - 1>;

// クラス型をテンプレート引数に指定できるようになるのはC++20以降
// ただし、クラスをテンプレート引数に指定できるためには、
// テンプレート引数に指定されているクラスがいくつかの条件を充たしている必要がある。
template<StringLiteral literal>
constexpr bool IsSame( char_cptr arg_option ) {
  auto [ ptr, length ] = literal.get();

  if ( IsSameN( ptr, arg_option, length ) == true ) { return true; }

  return false;
}

} // STRING

// class List;
template<size_type OPTION_NUM>
struct OptionString;

// オプションの長さを実装するクラス
// ただし、Tpは配列型でなければならないという制限を付けたい。
template<size_type OPTION_NUM>
struct OptionStrImpl {
  friend struct OptionString<OPTION_NUM>;
public:
  char_cptr values_[OPTION_NUM];

private:
  template<typename T, index_type... index_list, UTIL::if_nullp_c<
    std::is_pointer_v<std::remove_extent_t<T>>
  >* = nullptr>
  consteval OptionStrImpl( T const string_ptr, std::index_sequence<index_list...> ) : values_{ string_ptr[index_list] ... } {}
};

template<size_type OPTION_NUM>
struct OptionString : public OptionStrImpl<OPTION_NUM> {
  consteval OptionString( auto const option_strs[OPTION_NUM] ) : OptionStrImpl<OPTION_NUM>( option_strs, std::make_index_sequence<OPTION_NUM>() ) {};
};

template<typename T, size_type OPTION_NUM>
class List {
  using value_type = T;
  using ptr = T*;
  using const_value_type = T const;
  using const_ptr = const_value_type *;
private:
  constexpr static size_type option_num_ = OPTION_NUM;
  const_ptr head_;
  // OptionString<OPTION_NUM> head_ptrs_;

public:
  consteval List( T const ( & option_list )[OPTION_NUM] ) : head_( option_list ) {}
  // : heads_( option_list ) {}
  // , head_ptrs_( OptionString<OPTION_NUM>( option_list ) ) {}

  // ~List() = default;
  // List( List const & ) = default;
  // List & operator=( List const & ) = default;
  // List( List && ) = default;
  // List & operator=( List && ) = default;

  consteval auto num() const noexcept { return option_num_; }
  // consteval auto strHead( index_type const index ) const noexcept { return head_ptrs_.values_[index]; }

  // [[nodiscard]] consteval index_type discriminant( string_type const arg_option ) const {
  //   for ( size_t index = 0; index < option_num_; ++index ) {
  //     auto length = STRING::Length( arg_option );
  //     auto value = head_ptrs_.values_[index];
  //     // auto ptr = arg_option;
  //     // STRING::IsSameN( arg_option, heads_ptr_.values_[index], length );
  //     // if ( STRING::IsSameN( arg_option, heads_ptr_.values_[index], STRING::Length( heads_ptr_.values_[index] ) ) ) { return index; }
  //   }

  //   return option_num_;
  // }
};

// template<typename T, size_type N>
// consteval index_type discriminant( List<T, N> list, char_cptr const arg_option ) {
//   size_type const option_num = list.num();

//   for ( size_t index = 0; index < option_num; ++index ) {
//     size_type length = STRING::Length( arg_option );

//     if ( STRING::IsSameN( arg_option, list.strHead( index ), length ) ) {
//       return index;
//     }
//   }

//   return option_num;
// }

} // OPTION

} // ARG
