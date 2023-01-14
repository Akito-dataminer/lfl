#include "jig.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/interface.hpp>
#include <boost/test/tools/old/interface.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <utility>

#define OPTION_LIST { "directory", "help", "version" }

BOOST_AUTO_TEST_SUITE( test_jig )

BOOST_AUTO_TEST_CASE( test_meta_func_ArraySize ) {
  constexpr char const * option_list[] = OPTION_LIST;

  static_assert( jig::ArraySize( option_list ) == 3, "jig::ArraySize( option_list ) != 3" );

  constexpr char const option_str[] = "directory";
  // constexpr char const * option_str = "directory"; // this is errored.
  static_assert( jig::ArraySize( option_str ) == 10, "jig::ArraySize( option_str[] ) != 10" );

  static_assert( jig::ArraySize( "directory" ) == 10, "jig::ArraySize( \"directory\" ) != 10");

  // Below code is errored.
  // static_assert( jig::ArraySize<decltype( option_list ), 1>( option_list ) == 0, "jig::ArraySize( option_list ) != 0" );
  //
  // constexpr int i = 10;
  // static_assert( jig::ArraySize( i ) == 0 );
}

BOOST_AUTO_TEST_CASE( test_meta_func_Length ) {
  constexpr char const option_str[] = "directory";
  static_assert( jig::STRING::Length( option_str ) == 9, "jig::OPTION::STRING::Length( option_list[0] ) != 9" );

  STATIC_CONSTEXPR char const str[] = "directory";

  struct SendString {
    constexpr char const * operator()() { return str; }
  };
  static_assert( jig::STRING::Length<SendString>() == 9 );
}

BOOST_AUTO_TEST_CASE( test_make_literal_func ) {
  using namespace jig::STRING;
  STATIC_CONSTEXPR char const str[] = "directory";

  struct SendString {
    constexpr char const * operator()() { return str; }
  };

  constexpr auto literal = MakeString<SendString>();
  static_assert( literal.size() == 9 );
  static_assert( literal.get()[0] == 'd' );
}

BOOST_AUTO_TEST_CASE( test_meta_func_IsSameN ) {
  constexpr char const str1[] = "directory";
  constexpr char const str2[] = "directory";
  constexpr char const str3[] = "help";

  static_assert( jig::STRING::IsSameN( str1, str2, 9 ) == true, "jig::OPTION::STRING::IsSameN( str1, str2, 9 ) != true" );
  static_assert( jig::STRING::IsSameN( str1, str3, 9 ) == false, "jig::OPTION::STRING::IsSameN( str1, str3, 9 ) != false" );

  bool is_same_1_2 = jig::STRING::IsSameN( str1, str2, 9 );
  bool is_same_1_3 = jig::STRING::IsSameN( str1, str3, 9 );

  BOOST_CHECK( is_same_1_2 == true );
  BOOST_CHECK( is_same_1_3 == false );
}

BOOST_AUTO_TEST_CASE( constexpr_func_IsSame ) {
  using namespace jig::STRING;

  // This is error.
  // Literal<char, 0> literal0( "" );
  STATIC_CONSTEXPR Literal literal1( "directory" );

  static_assert( IsSame<literal1>( "directory" ) == true );

  char const * arg_option = "directory";

  bool const is_same = IsSame<literal1>( arg_option );

  BOOST_CHECK( is_same == true );
}

BOOST_AUTO_TEST_CASE( test_class_template_StringLiteral ) {
  using namespace jig::STRING;

  STATIC_CONSTEXPR char const * str1 = "directory";
  STATIC_CONSTEXPR Literal<char const *, Length(str1)> literal1( str1 );

  STATIC_CONSTEXPR char const * options[] = { "directory", "help" };
  constexpr Literal<char const *, Length(options[0])> literal2( options[0] );

  BOOST_CHECK( IsSame<literal1>( str1 ) == true );
  BOOST_CHECK( IsSame<literal2>( options[0] ) == true );
}

BOOST_AUTO_TEST_CASE( constexpr_func_LiteralIndex ) {
  using namespace jig;

  STATIC_CONSTEXPR STRING::Literal literal1( "directory" );
  STATIC_CONSTEXPR STRING::Literal literal2( "help" );

  char const * arg_dir = "directory";

  auto [ is_match_dir, index_dir ] = OPTION::LiteralIndex<literal1, literal2>( arg_dir );

  BOOST_CHECK( is_match_dir == true );
  BOOST_CHECK( index_dir == 0 );

  char const * arg_help = "help";
  auto [ is_match_help, index_help ] = OPTION::LiteralIndex<literal1, literal2>( arg_help );

  BOOST_CHECK( is_match_help == true );
  BOOST_CHECK( index_help == 1 );

  char const * arg_typo = "hop";
  auto [ non_match, index_non ] = OPTION::LiteralIndex<literal1, literal2>( arg_typo );

  BOOST_CHECK( non_match == false );
  BOOST_CHECK( index_non == 2 );
}

BOOST_AUTO_TEST_CASE( constexpr_func_GetOptionStr ) {
  using namespace jig;

  STATIC_CONSTEXPR STRING::Literal literal1( "directory" );
  STATIC_CONSTEXPR STRING::Literal literal2( "help" );

  STATIC_CONSTEXPR OPTION::OptionList<literal1, literal2> option;
  STATIC_CONSTEXPR auto literal( GetLiteral<0>( option ) );

  static_assert( STRING::IsSame<literal>( "directory" ) == true );
}

BOOST_AUTO_TEST_CASE( test_Option_isMatch ) {
  using namespace jig::OPTION;
  using namespace jig;

  OptionList<STRING::Literal( "directory" ), STRING::Literal( "help" )> options;

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
  using namespace jig::STRING;

  STATIC_CONSTEXPR char const option1[] = "directory";
  constexpr auto literal1 = ToStringLiteral<char, option1>();

  static_assert( literal1.size() == 9 );
}

BOOST_AUTO_TEST_CASE( test_MakeStringLiteral ) {
  using namespace jig::STRING;

  STATIC_CONSTEXPR char const option1[] = "directory";
  constexpr auto literal1 = ToStringLiteral<char, option1>();

  static_assert( literal1.size() == 9 );
}

BOOST_AUTO_TEST_CASE( test_OptionListisMatch ) {
  using namespace jig;
  using namespace jig::OPTION;
  using namespace jig::STRING;

  char const * arg_dir = "directory";
  char const * arg_help = "help";
  char const * arg_typo = "hlep";

  /******************************
  * Make Pattern 1
  *******************************/
  STATIC_CONSTEXPR char option1[] = "directory";
  STATIC_CONSTEXPR char option2[] = "help";

  OptionList<ToStringLiteral<char, option1>(), ToStringLiteral<char, option2>()> option_separate;

  auto [ ptr_dir_sep, length_dir_sep ] = option_separate.isMatch( arg_dir );
  BOOST_CHECK( ptr_dir_sep != nullptr );
  BOOST_CHECK( length_dir_sep == 9 );

  auto [ ptr_help_sep, length_help_sep ] = option_separate.isMatch( arg_help );
  BOOST_CHECK( ptr_help_sep != nullptr );
  BOOST_CHECK( length_help_sep == 4 );

  auto [ ptr_typo_sep, length_typo_sep ] = option_separate.isMatch( arg_typo );
  BOOST_CHECK( ptr_typo_sep == nullptr );
  BOOST_CHECK( length_typo_sep == 0 );

  /******************************
  * Make Pattern 2
  *******************************/
  STATIC_CONSTEXPR char const * options[] = { "directory", "help" };

  OptionList<
    STRING::Literal<char const *, STRING::Length(options[0])>( options[0] )
    , STRING::Literal<char const *, STRING::Length(options[1])>( options[1] )
  > option_list;

  auto [ ptr_dir, length_dir ] = option_list.isMatch( arg_dir );
  BOOST_CHECK( ptr_dir != nullptr );
  BOOST_CHECK( length_dir == 9 );

  auto [ ptr_help, length_help ] = option_list.isMatch( arg_help );
  BOOST_CHECK( ptr_help != nullptr );
  BOOST_CHECK( length_help == 4 );

  auto [ ptr_typo, length_typo ] = option_list.isMatch( arg_typo );
  BOOST_CHECK( ptr_typo == nullptr );
  BOOST_CHECK( length_typo == 0 );
}

BOOST_AUTO_TEST_CASE( test_OptionListMatchIndex ) {
  using namespace jig::OPTION;
  using namespace jig::STRING;

  char const * arg_dir = "directory";
  char const * arg_help = "help";
  char const * arg_typo = "hlep";

  /******************************
  * Make Pattern 1
  *******************************/
  STATIC_CONSTEXPR char option1[] = "directory";
  STATIC_CONSTEXPR char option2[] = "help";

  OptionList<ToStringLiteral<char, option1>(), ToStringLiteral<char, option2>()> option_separate;

  auto [ bool_dir_sep, index_dir_sep ] = option_separate.matchIndex( arg_dir );
  BOOST_CHECK( bool_dir_sep == true );
  BOOST_CHECK( index_dir_sep == 0 );

  auto [ bool_help_sep, index_help_sep ] = option_separate.matchIndex( arg_help );
  BOOST_CHECK( bool_help_sep == true );
  BOOST_CHECK( index_help_sep == 1 );

  auto [ bool_typo_sep, index_typo_sep ] = option_separate.matchIndex( arg_typo );
  BOOST_CHECK( bool_typo_sep == false );
  BOOST_CHECK( index_typo_sep == 2 );

  /******************************
  * Make Pattern 2
  *******************************/
  STATIC_CONSTEXPR char const * options[] = { "directory", "help" };

  OptionList<
    jig::STRING::Literal<char const *, jig::STRING::Length(options[0])>( options[0] )
    , jig::STRING::Literal<char const *, jig::STRING::Length(options[1])>( options[1] )
  > option_list;

  auto [ bool_dir, index_dir ] = option_list.matchIndex( arg_dir );
  BOOST_CHECK( bool_dir == true );
  BOOST_CHECK( index_dir == 0 );

  auto [ bool_help, index_help ] = option_list.matchIndex( arg_help );
  BOOST_CHECK( bool_help == true );
  BOOST_CHECK( index_help == 1 );

  auto [ bool_typo, index_typo ] = option_list.matchIndex( arg_typo );
  BOOST_CHECK( bool_typo == false );
  BOOST_CHECK( index_typo == 2 );
}

BOOST_AUTO_TEST_CASE( test_output_stream ) {
  using namespace jig;
  using namespace jig::STRING;

  Literal literal( "directory" );

  std::cout << literal << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
