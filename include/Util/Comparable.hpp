/****************************************
 * Util/Comparable.hpp
 ****************************************/
#pragma once

#include <utility>
#include <type_traits>

namespace UTIL {

namespace COMPARABLE {
/****************************************
 * <演算子が定義されているかを判定するためのメタ関数
 ****************************************/
struct is_lt_impl {
  template<class Left, class Right>
  static auto check( Left*, Right* ) -> decltype( std::declval<Left>() < std::declval<Right>(), std::true_type() );

  template<class Left, class Right>
  static auto check(...) -> std::false_type;
};

template<class Left, class Right>
struct has_less_than : public decltype( is_lt_impl::check<Left, Right>( nullptr, nullptr ) ){};

/****************************************
 * CRTPパターン
 *
 * <演算子が定義されていて、かつ、
 * <演算子がbool値を返すときに限り、コンパイルエラーを表示せずに
 * 他の比較演算子も定義するためのクラス
 *
 * <<演算子と<演算子をうっかり間違って定義していた場合などは
 * <演算子がbool以外の値を返す可能性があるので、警告を出す
 ****************************************/
template<class HasLT>
class CompDef {
public:
  using const_reference = HasLT const &;

  consteval CompDef() {
    // <演算子が定義されていなければコンパイルエラーを表示する
    static_assert(std::is_same<typename has_less_than<HasLT, HasLT>::type, std::true_type>::value, "operator < is NOT defined");

    // <演算子がbool値以外を返すように定義されていたらコンパイルエラーを表示する
    static_assert(std::is_same<decltype( std::declval<HasLT>() < std::declval<HasLT>() ), bool>::value, "operator < does NOT return bool value");
  }

  friend constexpr bool operator > ( const_reference c1, const_reference c2 ) noexcept { return( c2 < c1 ); }

  friend constexpr bool operator <= ( const_reference c1, const_reference c2 ) noexcept { return( !( c2 < c1 ) ); }
  friend constexpr bool operator >= ( const_reference c1, const_reference c2 ) noexcept { return( !( c1 < c2 ) ); }

  friend constexpr bool operator == ( const_reference c1, const_reference c2 ) noexcept { return( !( c1 < c2 ) && !( c2 < c1 ) ); }
  // friend constexpr bool operator != ( const_reference c1, const_reference c2 ) noexcept { return( !( c1 == c2 ) ); }
};

}; // COMPARABLE

}; // UTIL
