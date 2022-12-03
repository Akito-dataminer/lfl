#include "Argument.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/tools/old/interface.hpp>

#include <iostream>
#include <string>
#include <cstring>

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

  const bool is_same = IsSame<literal1>( arg_option );

  BOOST_CHECK( is_same == true );
}

BOOST_AUTO_TEST_CASE( test_argument_classes ) {
  // constexpr char const * option_list[] = OPTION_LIST;
  // constexpr char const * arg_option = "directory";
  // STATIC_CONSTEXPR ARG::OPTION::StringLiteral literals[] = { "directory", "help" };
  // STATIC_CONSTEXPR ARG::OPTION::List options( option_list );
  // STATIC_CONSTEXPR ARG::OPTION::List option_one_arg( "directory" );

  // static_assert( options.num() == 3 );

  // constexpr auto disc = ARG::OPTION::discriminant( options, "directory" );
  // constexpr auto disc2 = options.discriminant( "directory" );
  // static_assert( options.discriminant( "directory" ), "");
  // static_assert( disc != 0, "");

  // constexpr ARG::OPTION::List options_literal( { "directory", "help" } );
  // static_assert( options_literal.num() == 2 );
  // constexpr auto option_id = options.discriminate( option_list[0] );
  // std::cout << "option_id: " << option_id << std::endl;

  // BOOST_CHECK( options.num() == 3 );

  // BOOST_CHECK( options.discriminant( "directory" ) );

  // BOOST_CHECK( option_id == 0 );
  // BOOST_CHECK( std::strncmp( options.Discriminate(ARG::OPTION::option_list), option_list[0], 9 ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
