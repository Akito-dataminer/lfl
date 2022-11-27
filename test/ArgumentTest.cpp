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
  ARG::OPTION::List options;
  auto option_id = options.Discriminate( option_list[0] );
  std::cout << "option_id: " << option_id << std::endl;

  BOOST_CHECK( option_id == 0 );
  // BOOST_CHECK( std::strncmp( options.Discriminate(ARG::OPTION::option_list), option_list[0], 9 ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
