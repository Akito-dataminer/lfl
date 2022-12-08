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

BOOST_AUTO_TEST_CASE( test_class_template_StringLiteral ) {
  using namespace ARG::OPTION::STRING;

  STATIC_CONSTEXPR char const * str1 = "directory";
  STATIC_CONSTEXPR StringLiteral<char const *, Length(str1)> literal1( str1 );

  STATIC_CONSTEXPR char const * options[] = { "directory", "help" };
  constexpr StringLiteral<char const *, Length(options[0])> literal2( options[0] );

  BOOST_CHECK( IsSame<literal1>( str1 ) == true );
  BOOST_CHECK( IsSame<literal2>( options[0] ) == true );
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

  STATIC_CONSTEXPR OptionList<literal1, literal2> option;
  STATIC_CONSTEXPR auto literal( GetLiteral<0>( option ) );

  static_assert( STRING::IsSame<literal>( "directory" ) == true );
}

BOOST_AUTO_TEST_CASE( test_Option_isMatch ) {
  using namespace ARG::OPTION;
  using namespace ARG::OPTION::STRING;

  OptionList<STRING::StringLiteral( "directory" ), STRING::StringLiteral( "help" )> options;

  char const * arg_dir = "directory";
  auto [ head_ptr_dir, length_dir ] = options.isMatch( arg_dir );

  BOOST_TEST( length_dir == 9 );
  BOOST_TEST( head_ptr_dir != nullptr );
  BOOST_TEST( *head_ptr_dir == 'd' );

  char const * arg_help = "help";
  auto [ head_ptr_help, length_help ] = options.isMatch( arg_help );

  BOOST_TEST( length_help == 4 );
  BOOST_TEST( head_ptr_help != nullptr );
  BOOST_TEST( *head_ptr_help == 'h' );

  char const * arg_typo = "hop";
  auto [ head_ptr_typo, length_typo ] = options.isMatch( arg_typo );

  BOOST_TEST( length_typo == 0 );
  BOOST_TEST( head_ptr_typo == nullptr );
}

BOOST_AUTO_TEST_CASE( test_ToStringLiteral ) {
  using namespace ARG::OPTION;

  STATIC_CONSTEXPR char const option1[] = "directory";
  constexpr auto literal1 = ToStringLiteral<char, option1>();

  static_assert( literal1.size() == 9 );
}

BOOST_AUTO_TEST_SUITE_END()
