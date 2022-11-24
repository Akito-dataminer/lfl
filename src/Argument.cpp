/*****************************
 * Argument.cpp
 *****************************/
#include "Argument.hpp"

namespace ARG {


id_type Discriminate( string_type arg_option ) {
  id_type option_id = OPTION_NUM;

  for ( id_type id_index = 0; id_index < OPTION_NUM; ++id_index ) {
    int option_length = std::strlen( option_list[id_index] ) - 1;
    int arg_length = std::strlen( arg_option ) - 1;

    if ( arg_length < option_length ) { continue; }

    if ( !std::strncmp( arg_option, option_list[id_index], option_length ) ) {
      option_id = id_index;
      break;
    }
  }

  return option_id;
}

} // ARG
