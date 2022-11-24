/*****************************
 * lfl/Argument
 *****************************/

#pragma once

#include <cstring>
#include <type_traits>

#ifndef OPTION_LIST
#define OPTION_LIST {}
#endif

namespace ARG {

using string_type = char const *;
using list_type = string_type [];
using id_type = std::size_t;
// constexpr char const * option_specificator = { "--" };
constexpr list_type option_list = OPTION_LIST;
constexpr decltype( std::extent<decltype( option_list ), 0>::value ) OPTION_NUM = std::extent<decltype( option_list ), 0>::value;

id_type Discriminate( string_type arg_option );

} // ARG
