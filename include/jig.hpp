/*****************************
 * lfl/Argument
 *
 * コマンドは、他のコマンドと連携することでより大きな目的のために働く。
 *
 * 複数のコマンドが使われているのであれば、一つのバッチファイルや一つのシェルスクリプトファイル、一行で書いたシェルスクリプトですら、極めて繊細な制御があって初めて成り立つ有機体なのだ。
 *
 * そこで使われているコマンドはすべて、ある一つの大きな目標のために動作する。
 *
 * その目標を達成するためには、我々人間が様々なコマンドをオプションという指示書によって、その動作を方向付ける必要がある。
 *
 * 当然のことだ。そもそも目標を立てるのが我々人間なのだから。
 * オプションにより、どんな情報をどう処理してほしいのかを、我々がコマンドに伝えるのだ。
 *
 * 日本人なら、コマンド利用の様子(あるいは、コマンドそのものかもしれない)を擬人化して楽しむことだろう。
 *
 * コマンドに作業内容を伝え、そしてそのコマンド群をうまく組み合わせて、我々は何を行っているのだろうか？
 * そもそも、先ほど全くの説明なしに登場した「目標」とは何なのだろうか？
 *
 * コンピュータを取り巻く環境は様々で、コンピュータを利用する環境も、人も様々だ。
 * だからその答えは、人によって、そして環境によって様々に変化することだろう。だが大まかにはどれも一緒だ。それはつまり、情報を処理することだ。
 *
 * つまり、コマンド群を組み合わせ、そのコマンドに細かな処理の詳細を(オプションという形式で)伝えるとはつまり、情報の処理方法を特定している行為に他ならない。
 *
 * これはとても繊細で、かつとてもダイナミックな行為だ。一つでもコマンドの使い方を間違ってしまえば、我々が思い描いた通りの情報処理は行えないかもしれない。
 *
 * 原理的には、いくらでも間違いようはあったはずだ。引数を間違って指定していた可能性、使うコマンドを間違っていた可能性、コマンドとオプションの組を間違ってしまっていた可能性など、考えていけばいくらでも思いつくだろう(私は、grepに渡す引数のファイル名と正規表現の順番をよく間違える。当然エラーとなる)。
 *
 * しかし、その原理的には存在し得た間違う可能性を、一つ一つしらみつぶしに消していった後に残ったスクリプトこそ、我々が本当に必要としていた情報処理系であり、我々の日々の学習と経験が生み出した構造物なのだ。時間とともに、その構造物は時代遅れになるかもしれない。一行で書かれた単純なスクリプトなら、使われたその次の瞬間には必要なくなっているかもしれない。
 *
 * だが我々プログラマーの活動の本質はいつだって変わらない。我々プログラマーは、常に「我々の欲している情報処理システムをいかに組み上げればよいだろうか？」という問題に直面し、それを上手く解決できるような鮮やかな手腕を磨いているのだ(いつも鮮やかな方法で解決できるとは限らないが)。
 *
 * オプションとは、そのような活動の補佐を務めるような存在だと、私は考えている。
 *
 * 手に馴染むようなものであるべきだし、使い方が分かりやすくなっているべきだとも思う。ところで、機械分野では、そのような道具に名前が付けられ、他の道具とは一線を画する存在だと際立たせられている。
 *
 * その道具には、「治具(jig)」という名前が付けられている。私はオプションそれ自体の存在性格、そして、オプションの実装を補佐するという、このライブラリの存在性格から、この「治具(jig)」という名前がぴったりであると感じた。
 *
 * そこで私はこのライブラリに、二重の意味を込めて「治具(jig)」という名前を付けた。
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

namespace jig {

using char_cptr = char const *;
using list_type = char_cptr [];
using index_type = std::size_t;
using size_type = std::size_t;

template <typename T, size_type SIZE>
constexpr size_type ArraySize( T (&)[SIZE] ) { return SIZE; }

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
template<typename CharT, size_type N, UTIL::if_nullp_c<( N > 0 )>* = nullptr>
struct Literal {
  explicit consteval Literal( CharT const ( & string_literal )[N + 1] ) {
    std::copy_n( string_literal, N, str_chars_ );
  }

  Literal<CharT, N> ( Literal<CharT, N>  const & ) = default;
  Literal<CharT, N> & operator=( Literal<CharT, N>  const & ) = default;
  Literal<CharT, N> ( Literal<CharT, N> && ) = default;
  Literal<CharT, N> & operator=( Literal<CharT, N> && ) = default;

  CharT str_chars_[N]; // N is guaranteed greater than 0. It has no meanings str_chars_[0] when compiling. Because this array can't be changed it size and value later.

  consteval decltype( N ) size() const noexcept { return N; }
  consteval char_cptr get() const noexcept { return str_chars_; }
};

// ポインタ対応のための特殊化
template<typename CharT, size_type N, UTIL::if_nullp_c<( N > 0 )>* NPTR>
struct Literal<CharT const *, N, NPTR> {
  explicit consteval Literal( CharT const * string_literal_p ) {
    std::copy_n( string_literal_p, N, str_chars_ );
  }

  CharT str_chars_[N];

  consteval decltype( N ) size() const noexcept { return N; }
  consteval char_cptr get() const noexcept { return str_chars_; }
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
Literal( CharT const ( & literal )[N] ) -> Literal<CharT, N - 1>;

// template<typename CharTp, UTIL::if_nullp_c<std::is_pointer_v<CharTp>>* = nullptr>
// StringLiteral( CharTp literal ) -> StringLiteral<CharTp, Length( literal )>;

// クラス型をテンプレート引数に指定できるようになるのはC++20以降
// ただし、クラスをテンプレート引数に指定できるためには、
// テンプレート引数に指定されているクラスがいくつかの条件を充たしている必要がある。
template<Literal LITERAL>
constexpr bool IsSame( char_cptr const str ) {
  constexpr auto length = LITERAL.size();
  constexpr auto ptr = LITERAL.get();

  return ( IsSameN( ptr, str, length ) == true ) ? true : false;
}

} // STRING

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

// ポインタ型からStringLiteral型に変換する。
template<typename CharT, CharT const * Ptr, size_type length = STRING::Length( Ptr )>
consteval auto ToStringLiteral () {
  return STRING::Literal<CharT const *, length>( Ptr );
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

} // jig
