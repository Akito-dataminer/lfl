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
  explicit consteval StringLiteral( CharT const ( & string_literal )[N + 1] ) {
    std::copy_n( string_literal, N, str_chars_ );
  }

  CharT str_chars_[N];

  consteval decltype( N ) size() const noexcept { return N; }
  consteval std::pair<CharT const *, decltype( N )> get() const noexcept { return { str_chars_, N }; }
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
StringLiteral( CharT const ( & literal )[N] ) -> StringLiteral<CharT, N - 1>;

// クラス型をテンプレート引数に指定できるようになるのはC++20以降
// ただし、クラスをテンプレート引数に指定できるためには、
// テンプレート引数に指定されているクラスがいくつかの条件を充たしている必要がある。
template<StringLiteral LITERAL>
constexpr bool IsSame( char_cptr const str ) {
  auto [ ptr, length ] = LITERAL.get();

  return ( IsSameN( ptr, str, length ) == true ) ? true : false;
}

} // STRING

// テンプレート引数に与えられたStringLiteralの集合から、
// マッチするものがあれば、そのインデックスを返して、
// マッチするものが無ければfalseを返す(std::pairとして)。
//
// 多重定義の解決時に一致度が同じとなるような関数を複数定義したことになってしまう。
// そのため、StringLiteralの指定が一つだけのときの多重定義の一致度を高める必要がある。
// StringLiteralが一つだけのときの一致度を高めるための手段として、
// テンプレート引数の最後(nullptrの部分)で一致度が高くなるようにしている。
template<index_type INDEX, size_type OPTION_NUM, STRING::StringLiteral LITERAL, UTIL::if_nullp_c< INDEX == ( OPTION_NUM - 1 ) >* = nullptr>
inline constexpr std::pair<bool, index_type> LiteralIndexImpl( char_cptr const arg_str ) {
  return ( STRING::IsSame<LITERAL>( arg_str ) == true ) ? std::pair<bool, index_type>{ true, INDEX } : std::pair<bool, index_type>{ false, OPTION_NUM };
}

template<index_type INDEX, size_type OPTION_NUM, STRING::StringLiteral LITERAL_HEAD, STRING::StringLiteral... LITERAL_TAIL>
inline constexpr std::pair<bool, index_type> LiteralIndexImpl( auto const arg_str ) {
  return ( STRING::IsSame<LITERAL_HEAD>( arg_str ) == true ) ? std::pair<bool, index_type>{ true, INDEX } : LiteralIndexImpl<INDEX + 1, OPTION_NUM, LITERAL_TAIL...>( arg_str );
}

template<STRING::StringLiteral... LITERALS>
constexpr auto LiteralIndex( char_cptr const arg_str ) {
  return LiteralIndexImpl<0, sizeof...( LITERALS ), LITERALS...>( arg_str );
}

// template<size_type OPTION_INDEX, typename ... T>
// struct Literals;

// template<size_type OPTION_INDEX>
// struct Literals<OPTION_INDEX> {};

// template<size_type OPTION_INDEX, typename HeadT, typename ... TailT>
// struct Literals<OPTION_INDEX, HeadT, TailT ...> : public Literals<OPTION_INDEX + 1, TailT ...> {
//   HeadT value;
// };

// template<typename ... List>
// struct LiteralSet : public Literals<0, List ...> {};

// template<size_type OPTION_NUM>
// template<size_type OPTION_NUM, STRING::StringLiteral ... Literals>
// struct OptionString;

// template<size_type OPTION_NUM>
// struct OptionStrImpl {
//   friend struct OptionString<OPTION_NUM>;
// public:
//   std::tuple<> values_;

// private:
//   template<STRING::StringLiteral ... literals>
//   consteval OptionStrImpl( T const string_ptr, std::index_sequence<index_list...> ) : values_{ string_ptr[index_list] ... } {}
// };

// template<size_type OPTION_NUM, STRING::StringLiteral ... Literals>
// struct OptionString : public OptionStrImpl<OPTION_NUM> {
//   consteval OptionString( auto const option_strs[OPTION_NUM] ) : OptionStrImpl<OPTION_NUM>( option_strs, std::make_index_sequence<OPTION_NUM>() ) {};
// };

class OptionSet {
private:
  // constexpr static size_type option_num_ = sizeof...( literals );
  // std::tuple<> strs_;
  // OptionString<OPTION_NUM> head_ptrs_;

public:
  // template<typename CharT = char const, STRING::StringLiteral<CharT> ... literals>
  // consteval OptionSet() : strs_( std::make_index_sequence<sizeof...( literals )>(), literals... ) {}
  // consteval OptionSet() : strs_( std::make_tuple( literals ... ) ) {}

  // ~List() = default;
  // List( List const & ) = default;
  // List & operator=( List const & ) = default;
  // List( List && ) = default;
  // List & operator=( List && ) = default;

  // consteval auto num() const noexcept { return option_num_; }
  // consteval auto strHead( index_type const index ) const noexcept { return head_ptrs_.values_[index]; }

  // consteval index_type discriminant( string_type const arg_option ) const {
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

// template<typename CharT = char const[], STRING::StringLiteral<CharT> ... literals>
// class OptionSet() -> class OptionSet<char const[], literals...>;

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
