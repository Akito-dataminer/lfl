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
  static_assert( ARG::ArraySize<decltype( option_list ), 1>( option_list ) == 0, "ARG::ArraySize( option_list ) != 0" );

  constexpr char const option_str[] = "directory";
  // constexpr char const * option_str = "directory"; // this is errored.
  static_assert( ARG::ArraySize( option_str ) == 10, "ARG::ArraySize( option_str[] ) != 10" );

  static_assert( ARG::ArraySize( "directory" ) == 10, "ARG::ArraySize( \"directory\" ) != 10");

  // Below code is errored.
  // constexpr int i = 10;
  // static_assert( ARG::ArraySize( i ) == 0 );
}

BOOST_AUTO_TEST_CASE( test_meta_func_Length ) {
  constexpr char const option_str[] = "directory";
  static_assert( ARG::OPTION::STRING::Length( option_str ) == 9, "ARG::OPTION::STRING::OptionLength( option_list[0] ) != 9" );
}

BOOST_AUTO_TEST_CASE( test_argument_classes ) {
  constexpr char const * option_list[] = OPTION_LIST;
  constexpr ARG::OPTION::List options( option_list );

  static_assert( options.num() == 3, "options.num() != 3");
  // static_assert( options.getLength( 0 ) == 10, "options.getLength( 0 ) != 3");

  // constexpr ARG::OPTION::List options_literal( { "directory", "help" } );
  // constexpr auto option_id = options.Discriminate( option_list[0] );
  // std::cout << "option_id: " << option_id << std::endl;

  BOOST_CHECK( options.num() == 3 );
  BOOST_CHECK( options.getLength( 0 ) == 9 );
  BOOST_CHECK( options.getLength( 1 ) == 4 );
  BOOST_CHECK( options.getLength( 2 ) == 7 );

  // BOOST_CHECK( option_id == 0 );
  // BOOST_CHECK( std::strncmp( options.Discriminate(ARG::OPTION::option_list), option_list[0], 9 ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
