set( LICENSE_HOLDER "Akito Matsumoto" )

if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
  find_package(Git)

  if( GIT_FOUND )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=4
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE "TAG_AND_BUILD_STRING"
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  else( GIT_FOUND )
    set( TAG_AND_BUILD_STRING "" )
  endif( GIT_FOUND )
endif()

configure_file( version.h.in ${CMAKE_SOURCE_DIR}/include/version.h )
