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

  StringLiteral<CharT, N> ( StringLiteral<CharT, N>  const & ) = default;
  StringLiteral<CharT, N> & operator=( StringLiteral<CharT, N>  const & ) = default;
  StringLiteral<CharT, N> ( StringLiteral<CharT, N> && ) = default;
  StringLiteral<CharT, N> & operator=( StringLiteral<CharT, N> && ) = default;

  CharT str_chars_[N];

  consteval decltype( N ) size() const noexcept { return N; }
  consteval std::pair<CharT const *, decltype( N )> get() const noexcept { return { str_chars_, N }; }
};

// ポインタ対応のための特殊化
template<typename CharT, size_type N>
struct StringLiteral<CharT const *, N> {
  explicit consteval StringLiteral( CharT const * string_literal_p ) {
    std::copy_n( string_literal_p, N, str_chars_ );
  }

  CharT str_chars_[N];

  consteval decltype( N ) size() const noexcept { return N; }
  consteval std::pair<CharT const *, decltype( N )> get() const noexcept { return { str_chars_, N }; }
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
StringLiteral( CharT const ( & literal )[N] ) -> StringLiteral<CharT, N - 1>;

// template<typename CharTp, UTIL::if_nullp_c<std::is_pointer_v<CharTp>>* = nullptr>
// StringLiteral( CharTp literal ) -> StringLiteral<CharTp, Length( literal )>;

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

// テンプレート引数に与えられた文字列リテラルを、
// タプルのように保存するためのテンプレートクラス
template<index_type INDEX, STRING::StringLiteral... LITERAL_TAIL>
struct OptionsImpl;

template<index_type INDEX>
struct OptionsImpl<INDEX> {};

template<index_type INDEX, STRING::StringLiteral LITERAL_HEAD, STRING::StringLiteral... LITERAL_TAIL>
struct OptionsImpl<INDEX, LITERAL_HEAD, LITERAL_TAIL...> : public OptionsImpl<INDEX + 1, LITERAL_TAIL...> {
  consteval decltype( LITERAL_HEAD ) const & literal() const noexcept { return LITERAL_HEAD; }

  constexpr auto isMatch( char const * str ) {
    if constexpr ( sizeof...( LITERAL_TAIL ) == 0 ) {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? LITERAL_HEAD.get()
        : decltype( LITERAL_HEAD.get() ) { nullptr, 0 };
    } else {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? LITERAL_HEAD.get()
        : OptionsImpl<INDEX + 1, LITERAL_TAIL...>::isMatch( str );
    }
  }
};

template<STRING::StringLiteral ... LITERALS>
struct OptionList : public OptionsImpl<0, LITERALS...> {
  template<std::make_index_sequence<sizeof...( LITERALS )>()>
  constexpr auto discriminant( char const * str ) { return OptionsImpl<0, LITERALS...>::isMatch( str ); }
};

// 上記テンプレートクラスから文字列をコンパイル時に受け取るための即時関数
template<index_type INDEX, STRING::StringLiteral LITERAL_HEAD, STRING::StringLiteral... LITERAL_TAIL>
inline consteval decltype( LITERAL_HEAD ) const & GetStringLiteral( OptionsImpl<INDEX, LITERAL_HEAD, LITERAL_TAIL...> const & option ) { return option.literal(); }

template<index_type INDEX, STRING::StringLiteral... STRING_LITERALS>
inline consteval auto GetLiteral( OptionList<STRING_LITERALS...> const & options ) -> decltype( GetStringLiteral<INDEX>( options ) ) { return GetStringLiteral<INDEX>( options ); }

// ポインタ型からStringLiteral型に変換する。
template<typename CharT, CharT const * ptr, size_type length = STRING::Length( ptr )>
consteval auto ToStringLiteral () {
  return STRING::StringLiteral<CharT const *, length>( ptr );
}

// template<typename CharTp>
// struct MakeOptionListImpl {
//   OptionList<> literals_;

//   template<index_type... indices>
//   MakeOptionListImpl( CharTp const literal_ptrs, std::index_sequence<indices...> )
//   : literals_( STRING::StringLiteral<char const *, STRING::Length( literal_ptrs[indices] )>()... ) {}
// };

// template<typename CharTp>
// struct OptionsList : public MakeOptionListImpl<CharTp> {
//   template<typename T, size_type NUM>
//   explicit consteval OptionsList( T const literal_ptrs ) : MakeOptionListImpl<CharTp>( literal_ptrs, std::make_index_sequence<NUM>() ) {}
// };

// 複数のStringLiteralを扱うためのクラス
//
// char const * option_strs[] = { "directory", "help" }
// OptionList<option_strs> option_list;
// auto [ is_match, index ] = option_list.discriminant();
//
// のようなことをしたい
// template<typename Tp, size_type OPTION_NUM, UTIL::if_nullp_c<std::is_pointer_v<Tp>>* = nullptr>
// struct MakeOptionList {
//   explicit consteval MakeOptionList( Tp const ( & literal_array )[OPTION_NUM] ) : options_( literal_array, OPTION_NUM ) {}

//   OptionsList<Tp> options_;

//   index_type discriminant() {}
// };

// 現状では、テンプレートパラメータとして与えられていないstringsを
// テンプレートパラメータに使おうとしているので、
// これをインスタンス化したら、その瞬間にエラーになる。
// template<typename Tp, index_type... INDICES>
// inline consteval auto MakeOptionListImpl( Tp const strings, std::index_sequence<INDICES...> ) {
//   using namespace STRING;
//   return OptionList<StringLiteral<std::remove_pointer<Tp> const *, Length( strings[INDICES] )>( strings[INDICES] ) ...>();
// }

// template<typename Tp, size_type OPTION_NUM, UTIL::if_nullp_c<std::is_pointer_v<Tp>>* = nullptr>
// inline consteval auto MakeOptionList( Tp const ( & literal_array )[OPTION_NUM] ) {
//   return MakeOptionListImpl( literal_array, std::make_index_sequence<OPTION_NUM>() );
// }

// template<typename CharT, size_type N>
// StringLiteral( CharT const ( & literal )[N] ) -> StringLiteral<CharT, N - 1>;

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
