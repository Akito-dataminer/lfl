#include "jig.hpp"

#include <string>
#include <cstring>
#include <utility>
#include <type_traits>
#include <algorithm>

namespace jig {

namespace OPTION {
// テンプレート引数に与えられたStringLiteralの集合から、
// マッチするものがあれば、そのインデックスを返して、
// マッチするものが無ければfalseを返す(std::pairとして)。
//
// 多重定義の解決時に一致度が同じとなるような関数を複数定義したことになってしまう。
// そのため、StringLiteralの指定が一つだけのときの多重定義の一致度を高める必要がある。
// StringLiteralが一つだけのときの一致度を高めるための手段として、
// テンプレート引数の最後(nullptrの部分)で一致度が高くなるようにしている。
template<index_type INDEX, size_type OPTION_NUM, STRING::Literal LITERAL, UTIL::if_nullp_c< INDEX == ( OPTION_NUM - 1 ) >* = nullptr>
inline constexpr std::pair<bool, index_type> LiteralIndexImpl( char_cptr const arg_str ) {
  return ( STRING::IsSame<LITERAL>( arg_str ) == true ) ? std::pair<bool, index_type>{ true, INDEX } : std::pair<bool, index_type>{ false, OPTION_NUM };
}

template<index_type INDEX, size_type OPTION_NUM, STRING::Literal LITERAL_HEAD, STRING::Literal... LITERAL_TAIL>
inline constexpr std::pair<bool, index_type> LiteralIndexImpl( auto const arg_str ) {
  return ( STRING::IsSame<LITERAL_HEAD>( arg_str ) == true ) ? std::pair<bool, index_type>{ true, INDEX } : LiteralIndexImpl<INDEX + 1, OPTION_NUM, LITERAL_TAIL...>( arg_str );
}

template<STRING::Literal... LITERALS>
constexpr auto LiteralIndex( char_cptr const arg_str ) {
  return LiteralIndexImpl<0, sizeof...( LITERALS ), LITERALS...>( arg_str );
}

// テンプレート引数に与えられた文字列リテラルを、
// タプルのように保存するためのテンプレートクラス
template<index_type INDEX, STRING::Literal... LITERAL_TAIL>
struct OptionsImpl;

template<index_type INDEX>
struct OptionsImpl<INDEX> {};

template<index_type INDEX, STRING::Literal LITERAL_HEAD, STRING::Literal... LITERAL_TAIL>
struct OptionsImpl<INDEX, LITERAL_HEAD, LITERAL_TAIL...> : public OptionsImpl<INDEX + 1, LITERAL_TAIL...> {
  consteval decltype( LITERAL_HEAD ) const & literal() const noexcept { return LITERAL_HEAD; }

  constexpr auto isMatch( char const * str ) {
    if constexpr ( sizeof...( LITERAL_TAIL ) == 0 ) {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? std::pair<char_cptr, size_type>{ LITERAL_HEAD.get(), LITERAL_HEAD.size() } : std::pair<char_cptr, size_type>{ nullptr, 0 };
    } else {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? std::pair<char_cptr, size_type>{ LITERAL_HEAD.get(), LITERAL_HEAD.size() } : OptionsImpl<INDEX + 1, LITERAL_TAIL...>::isMatch( str );
    }
  }

  constexpr auto matchIndex( char const * str ) {
    if constexpr ( sizeof...( LITERAL_TAIL ) == 0 ) {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? std::pair<bool, index_type>{ true, INDEX }
        : std::pair<bool, index_type>{ false, INDEX + 1 };
    } else {
      return ( STRING::IsSame<LITERAL_HEAD>( str ) == true )
        ? std::pair<bool, index_type>{ true, INDEX }
        : OptionsImpl<INDEX + 1, LITERAL_TAIL...>::matchIndex( str );
    }
  }
};

template<STRING::Literal ... LITERALS>
struct OptionList : public OptionsImpl<0, LITERALS...> {};

// 上記テンプレートクラスから文字列をコンパイル時に受け取るための即時関数
template<index_type INDEX, STRING::Literal LITERAL_HEAD, STRING::Literal... LITERAL_TAIL>
inline consteval decltype( LITERAL_HEAD ) const & GetStringLiteral( OptionsImpl<INDEX, LITERAL_HEAD, LITERAL_TAIL...> const & option ) { return option.literal(); }

template<index_type INDEX, STRING::Literal... STRING_LITERALS>
inline consteval auto GetLiteral( OptionList<STRING_LITERALS...> const & options ) -> decltype( GetStringLiteral<INDEX>( options ) ) { return GetStringLiteral<INDEX>( options ); }

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

} // jig
