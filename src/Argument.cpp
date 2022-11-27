/*****************************
 * Argument.cpp
 *****************************/
#include "Argument.hpp"

namespace ARG {

namespace OPTION {

index_type List::Discriminate( string_type arg_option ) {
  index_type option_id = option_num;

  for ( index_type id_index = 0; id_index < option_num; ++id_index ) {
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

length_type List::countLength() { return 0; }

} // OPTION


} // ARG
