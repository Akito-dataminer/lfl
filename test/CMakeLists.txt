add_subdirectory( Util )

set( TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/test )
include( ${TEST_DIRECTORY}/AddTestHelpers.cmake )

## テストケースの追加
set( TEST_NAME1 test_jig )
set( SOURCE_PATH jigTest.cpp )
create_executable( ${TEST_NAME1} ${SOURCE_PATH} )

add_test(
  NAME ${TEST_NAME1}
  COMMAND ${TEST_NAME1}
  )

