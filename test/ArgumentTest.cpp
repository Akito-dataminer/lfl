#define OPTION_LIST { "directory", "help" }

#include "Argument.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/tools/old/interface.hpp>

#include <iostream>
#include <string>
#include <cstring>

BOOST_AUTO_TEST_SUITE( test_util )

BOOST_AUTO_TEST_CASE( test_compare_assert ) {
  char const * option_list[] = OPTION_LIST;
  decltype( ARG::Discriminate( ARG::option_list[0] ) ) option_id = ARG::Discriminate( ARG::option_list[0] );

  BOOST_CHECK( option_id == 0 );
  BOOST_CHECK( std::strncmp( ARG::option_list[option_id], option_list[0], 9 ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
