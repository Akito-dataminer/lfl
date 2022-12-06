#include "Argument.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/tools/old/interface.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <utility>

#define OPTION_LIST { "directory", "help", "version" }

BOOST_AUTO_TEST_SUITE( test_Argument )

BOOST_AUTO_TEST_CASE( test_meta_func_ArraySize ) {
  constexpr char const * option_list[] = OPTION_LIST;

  static_assert( ARG::ArraySize( option_list ) == 3, "ARG::ArraySize( option_list ) != 3" );

  constexpr char const option_str[] = "directory";
  // constexpr char const * option_str = "directory"; // this is errored.
  static_assert( ARG::ArraySize( option_str ) == 10, "ARG::ArraySize( option_str[] ) != 10" );

  static_assert( ARG::ArraySize( "directory" ) == 10, "ARG::ArraySize( \"directory\" ) != 10");

  // Below code is errored.
  // static_assert( ARG::ArraySize<decltype( option_list ), 1>( option_list ) == 0, "ARG::ArraySize( option_list ) != 0" );
  //
  // constexpr int i = 10;
  // static_assert( ARG::ArraySize( i ) == 0 );
}

BOOST_AUTO_TEST_CASE( test_meta_func_Length ) {
  constexpr char const option_str[] = "directory";
  static_assert( ARG::OPTION::STRING::Length( option_str ) == 9, "ARG::OPTION::STRING::Length( option_list[0] ) != 9" );
}

BOOST_AUTO_TEST_CASE( test_meta_func_IsSameN ) {
  constexpr char const str1[] = "directory";
  constexpr char const str2[] = "directory";
  constexpr char const str3[] = "help";

  static_assert( ARG::OPTION::STRING::IsSameN( str1, str2, 9 ) == true, "ARG::OPTION::STRING::IsSameN( str1, str2, 9 ) != true" );
  static_assert( ARG::OPTION::STRING::IsSameN( str1, str3, 9 ) == false, "ARG::OPTION::STRING::IsSameN( str1, str3, 9 ) != false" );

  bool is_same_1_2 = ARG::OPTION::STRING::IsSameN( str1, str2, 9 );
  bool is_same_1_3 = ARG::OPTION::STRING::IsSameN( str1, str3, 9 );

  BOOST_CHECK( is_same_1_2 == true );
  BOOST_CHECK( is_same_1_3 == false );
}

BOOST_AUTO_TEST_CASE( constexpr_func_IsSame ) {
  using namespace ARG::OPTION::STRING;

  STATIC_CONSTEXPR StringLiteral literal1( "directory" );

  static_assert( IsSame<literal1>( "directory" ) == true );

  char const * arg_option = "directory";

  bool const is_same = IsSame<literal1>( arg_option );

  BOOST_CHECK( is_same == true );
}

BOOST_AUTO_TEST_CASE( constexpr_func_LiteralIndex ) {
  using namespace ARG::OPTION;

  STATIC_CONSTEXPR STRING::StringLiteral literal1( "directory" );
  STATIC_CONSTEXPR STRING::StringLiteral literal2( "help" );

  char const * arg_dir = "directory";

  auto [ is_match_dir, index_dir ] = LiteralIndex<literal1, literal2>( arg_dir );

  BOOST_CHECK( is_match_dir == true );
  BOOST_CHECK( index_dir == 0 );

  char const * arg_help = "help";
  auto [ is_match_help, index_help ] = LiteralIndex<literal1, literal2>( arg_help );

  BOOST_CHECK( is_match_help == true );
  BOOST_CHECK( index_help == 1 );

  char const * arg_typo = "hop";
  auto [ non_match, index_non ] = LiteralIndex<literal1, literal2>( arg_typo );

  BOOST_CHECK( non_match == false );
  BOOST_CHECK( index_non == 2 );
}

BOOST_AUTO_TEST_CASE( constexpr_func_GetOptionStr ) {
  using namespace ARG::OPTION;

  STATIC_CONSTEXPR STRING::StringLiteral literal1( "directory" );
  STATIC_CONSTEXPR STRING::StringLiteral literal2( "help" );

  STATIC_CONSTEXPR Options<literal1, literal2> option;
  STATIC_CONSTEXPR auto literal( GetOptionStr<0>( option ) );

  static_assert( STRING::IsSame<literal>( "directory" ) == true );
}

BOOST_AUTO_TEST_CASE( test_argument_classes ) {
  using namespace ARG::OPTION;
  using namespace ARG::OPTION::STRING;

  // STATIC_CONSTEXPR StringLiteral literal1( "directory" );
  // STATIC_CONSTEXPR StringLiteral literal2( "help" );

  // std::tuple options( literal1, literal2 );

  // auto options = std::make_tuple( StringLiteral( "directory" ), StringLiteral( "help" ) );
  // OptionSet literal_set< StringLiteral( "directory" ), StringLiteral( "help" ) >();
  // LiteralSet literals( StringLiteral( "directory" ), StringLiteral( "help" ) );
  // SetImpl options( literal1, literal2 );
}

BOOST_AUTO_TEST_SUITE_END()
