#include "Util/Comparable.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/tools/old/interface.hpp>

#include <iostream>
#include <string>

BOOST_AUTO_TEST_SUITE( test_util )

using namespace UTIL;

class IntClass : public COMPARABLE::CompDef<IntClass> {
// class IntClass {
private:
  int i_;
public:
  IntClass( int const i ) : UTIL::COMPARABLE::CompDef<IntClass>(), i_( i ) {}

  friend bool operator < ( IntClass const & i1, IntClass const & i2 ) noexcept { return( i1.i_ < i2.i_ ); }
};

class NotHaveLT : COMPARABLE::CompDef<NotHaveLT> {
public:
};
// class X {};

class LTReturnNONbool : COMPARABLE::CompDef<LTReturnNONbool> {
public:
  // 間違った戻り値指定
  friend std::string operator < ( LTReturnNONbool const & nonb1, LTReturnNONbool const & nonb2 ) noexcept { return( "operator" ); }
};

BOOST_AUTO_TEST_CASE( test_compare_assert ) {
  IntClass minimum(0), non_minimum(1);

  // NotHaveLT x; // ここでXが実体化されるため、コンパイルエラーが起こる
  // LTReturnNONbool non_bool; // 間違った戻り値指定をしているクラスを実体化しようとしているため、コンパイルエラーが起こる

  // static_assertは指定した定数式が偽のときにコンパイルエラーを生成する
  static_assert(COMPARABLE::has_less_than<IntClass, IntClass>::value, "IntClass does NOT have operator <");
  static_assert(!( COMPARABLE::has_less_than<NotHaveLT, NotHaveLT>::value ), "X < X is NOT defined");

  static_assert(!COMPARABLE::has_less_than<NotHaveLT, IntClass>::value, "X < IntClass is NOT defined");
}

// プロパティベースのテスト
// 演算子が6つあり、それぞれの演算子が2つのbool値を取るので、
// 本当なら、2^6個のテストが必要
BOOST_AUTO_TEST_CASE( test_compare_correctness ) {
  IntClass minimum(0), non_minimum(1);

  // 左側が小さいパターン
  BOOST_CHECK( minimum < non_minimum == 1 );
  BOOST_CHECK( minimum > non_minimum == 0 );
  BOOST_CHECK( minimum <= non_minimum == 1 );
  BOOST_CHECK( minimum >= non_minimum == 0 );
  BOOST_CHECK( minimum == non_minimum == 0 );
  BOOST_CHECK( minimum != non_minimum == 1 );

  // 左側が小さくないパターン
  BOOST_CHECK( non_minimum < non_minimum == 0 );
  BOOST_CHECK( non_minimum > non_minimum == 0 );
  BOOST_CHECK( non_minimum <= non_minimum == 1 );
  BOOST_CHECK( non_minimum >= non_minimum == 1 );
  BOOST_CHECK( non_minimum == non_minimum == 1 );
  BOOST_CHECK( non_minimum != non_minimum == 0 );

  // 左側が大きいパターン
  BOOST_CHECK( non_minimum < minimum == 0 );
  BOOST_CHECK( non_minimum > minimum == 1 );
  BOOST_CHECK( non_minimum <= minimum == 0 );
  BOOST_CHECK( non_minimum >= minimum == 1 );
  BOOST_CHECK( non_minimum == minimum == 0 );
  BOOST_CHECK( non_minimum != minimum == 1 );
}

BOOST_AUTO_TEST_SUITE_END()
