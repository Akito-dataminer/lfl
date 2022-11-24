add_subdirectory( Util )

set( TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/test )
include( ${TEST_DIRECTORY}/AddTestHelpers.cmake )

## テストケースの追加
set( TEST_NAME1 ArgumentTest.cpp )
set( SOURCE_PATH ${TEST_DIRECTORY}/${test_comparable} )
create_executable( ${TEST_NAME1} ${SOURCE_PATH} )

add_test(
  NAME ${TEST_NAME1}
  COMMAND ${TEST_NAME1}
  )

