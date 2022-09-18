## ヘルパー関数を利用するために、スクリプトファイルをインクルード
set( TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/test )
include( ${TEST_DIRECTORY}/AddTestHelpers.cmake )

## テストケースの追加
set( TEST_COMPARABLE test_comparable )
create_executable( ${TEST_COMPARABLE} Comparable.cpp Util )

add_test(
  NAME ${TEST_COMPARABLE}
  COMMAND ${TEST_COMPARABLE}
  )
