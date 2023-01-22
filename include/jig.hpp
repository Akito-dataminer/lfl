/*****************************
 * lfl/Argument
 *
 * Copyright (c) 2022 Akito-Dataminer
 * Released under the MIT license.
 * https://github.com/Akito-dataminer/lfl/blob/main/LICENSE.md
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

#include "Util/Comparable.hpp"

#include <string>
#include <stdexcept>
#include <cstring>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <ostream>

#define STATIC_CONSTEXPR static constexpr
#define STATIC_CONSTINIT static constinit

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

template<class T>
concept HasFunctionCall = requires() {
  T();
};

template<HasFunctionCall StringProxy>
consteval index_type Length() {
  index_type index = 0;
  while ( StringProxy()()[index] != '\0' ) { ++index; }
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

// N is guaranteed greater than 0. It has no meanings str_chars_[0] when compiling.
// Because this array can't be changed it size and value later.
template<typename CharT, size_type N>
struct ExcludeNULLLiteralImpl : public UTIL::COMPARABLE::CompDef<ExcludeNULLLiteralImpl<CharT, N>> {
  using value_type = CharT;
  using const_pointer = CharT const *;
  using reference = CharT &;
  using const_reference = CharT const &;
  using iterator = value_type *;
  using const_iterator = value_type const *;
  using difference_type = std::ptrdiff_t;

  explicit consteval ExcludeNULLLiteralImpl() : str_{ '\0' }, len_( 0 ) {}

  explicit consteval ExcludeNULLLiteralImpl( CharT const ( & literal )[N + 1] ) : len_( N ) { std::copy_n( literal, N, str_ ); }

  template<size_type... INDICES>
  explicit consteval ExcludeNULLLiteralImpl( CharT const * literal_p, std::index_sequence<INDICES...> ) : str_{ literal_p[INDICES] ... }, len_( N ) {}

  value_type str_[N];
  decltype( N ) len_;

  constexpr const_pointer get() const noexcept { return str_; }
  constexpr size_type size() const noexcept { return N; }

  constexpr reference operator[] ( size_type index ) noexcept { return str_[index]; }
  constexpr const_reference operator[] ( size_type index ) const noexcept { return str_[index]; }

protected:
  constexpr iterator makeIterator( size_type const index ) { return ( ( index < ( N + 1 ) ) ? ( str_ + index ) : ( throw std::out_of_range("makeIterator: out of range") ) ); }
  constexpr const_iterator makeConstIterator( size_type const index ) const { return ( ( index < ( N + 1 ) ) ? ( str_ + index ) : ( throw std::out_of_range("makeConstIterator: out of range") ) ); }
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
ExcludeNULLLiteralImpl( CharT const ( & literal )[N], std::make_index_sequence<N>() ) -> ExcludeNULLLiteralImpl<CharT, N - 1>;

////////////////////
// operator<
////////////////////
template<typename CharT, size_type N1, size_type N2>
constexpr bool operator< ( ExcludeNULLLiteralImpl<CharT, N1> const & literal1, ExcludeNULLLiteralImpl<CharT, N2> const & literal2 ) {
  auto len1 = literal1.len_;
  auto len2 = literal2.len_;

  for ( size_type index = 0; index < std::min( len1, len2 ); ++index ) {
    if ( literal1[index] < literal2[index] ) { return true; }
  }

  return ( len1 < len2 );
}

template<typename CharT, size_type N1, size_type N2>
constexpr bool operator< ( ExcludeNULLLiteralImpl<CharT, N1> const & literal1, CharT const ( & literal2 )[N2] ) {
  auto len1 = literal1.len_;

  for ( size_t index = 0; index < std::min( len1, N2 ); ++index ) {
    if ( literal1[index] < literal2[index] ) { return true; }
  }

  return ( len1 < ( N2 - 1 ) );
}

template<typename CharT, size_type N1, size_type N2>
constexpr bool operator< ( CharT const ( & literal1 )[N1], ExcludeNULLLiteralImpl<CharT, N2> const & literal2 ) {
  auto len2 = literal2.len_;

  for ( size_t index = 0; index < std::min( N1, len2 ) ; ++index ) {
    if ( literal1[index] <  literal2[index] ) { return true; }
  }

  return ( ( N1 - 1 ) < len2 );
}

// 最後の'\0'は含めたくない。
// というよりも、コンパイル時に要素数も分かるから必要ない。
template<typename CharT, size_type N, UTIL::if_nullp_c<( N > 0 )>* = nullptr>
struct Literal : public ExcludeNULLLiteralImpl<CharT, N> {
  using impl_type = ExcludeNULLLiteralImpl<CharT, N>;

  explicit consteval Literal() : ExcludeNULLLiteralImpl<CharT, N>() {}
  explicit consteval Literal( CharT const ( & string_literal )[N + 1] ) : ExcludeNULLLiteralImpl<CharT, N>( string_literal, std::make_index_sequence<N>() ) {}

  Literal<CharT, N> ( Literal<CharT, N>  const & ) = default;
  Literal<CharT, N> & operator=( Literal<CharT, N> const & ) = default;
  Literal<CharT, N> ( Literal<CharT, N> && ) = default;
  Literal<CharT, N> & operator=( Literal<CharT, N> && ) = default;

  constexpr typename impl_type::iterator begin() noexcept { return impl_type::makeIterator( 0 ); }
  constexpr typename impl_type::iterator end() noexcept { return impl_type::makeIterator( impl_type::len_ ); }
  constexpr typename impl_type::const_iterator cbegin() const noexcept { return impl_type::makeConstIterator( 0 ); }
  constexpr typename impl_type::const_iterator cend() const noexcept { return impl_type::makeConstIterator( impl_type::len_ ); }
};

template<typename CharT, size_type N>
inline STATIC_CONSTEXPR std::basic_ostream<CharT>& operator << ( std::basic_ostream<CharT> & lhs, Literal<CharT, N> const & rhs ) {
  return lhs << rhs.get();
}

// ポインタ対応のための特殊化
template<typename CharT, size_type N, UTIL::if_nullp_c<( N > 0 )>* NPTR>
struct Literal<CharT const *, N, NPTR> : public ExcludeNULLLiteralImpl<CharT, N> {
  using impl_type = ExcludeNULLLiteralImpl<CharT, N>;
  explicit consteval Literal( CharT const * string_literal_p ) : ExcludeNULLLiteralImpl<CharT, N>( string_literal_p, std::make_index_sequence<N>() ) {}
};

// 末尾の'\0'を含めないようにするための補助推論(補助推論はC++17から)
template<typename CharT, size_type N>
Literal( CharT const ( & literal )[N] ) -> Literal<CharT, N - 1>;

template<HasFunctionCall StringProxy>
consteval auto MakeString() {
  return Literal<decltype( StringProxy()() ), Length<StringProxy>()>( StringProxy()() );
}

// クラス型をテンプレート引数に指定できるようになるのはC++20以降
// ただし、クラスをテンプレート引数に指定できるためには、
// テンプレート引数に指定されているクラスがいくつかの条件を充たしている必要がある。
template<Literal LITERAL>
constexpr bool IsSame( char_cptr const str ) {
  constexpr auto length = LITERAL.size();
  constexpr auto ptr = LITERAL.get();

  return ( IsSameN( ptr, str, length ) == true ) ? true : false;
}

// ポインタ型からStringLiteral型に変換する。
template<typename CharT, CharT const * Ptr, size_type length = STRING::Length( Ptr )>
consteval auto ToStringLiteral () {
  return STRING::Literal<CharT const *, length>( Ptr );
}

} // STRING

} // jig
