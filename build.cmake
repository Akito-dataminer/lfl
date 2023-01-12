cmake_minimum_required( VERSION 3.25.1 )

set( build_dir ${CMAKE_CURRENT_LIST_DIR}/build )

if( NOT EXISTS ${build_dir} )
  file( MAKE_DIRECTORY ${build_dir} )
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  WORKING_DIRECTORY ${build_dir}
)

execute_process(
  COMMAND ${CMAKE_COMMAND} --build . -j8
  WORKING_DIRECTORY ${build_dir}
)
