set( MAIN_FILE_NAME Main.cpp ) # エントリポイントが書かれているファイルの名前
set( MAIN_FILE_DIRECTORY ${PROJECT_SOURCE_DIR}/src ) # エントリポイントが書かれているファイルの名前
set( MAIN_PATH ${MAIN_FILE_DIRECTORY}/${MAIN_FILE_NAME} )

set( TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/test )
set( TEST_MAIN TestMain.cpp )

##############################
## テストケースを追加するための関数
##############################
function( create_executable TEST_NAME TEST_SOURCE_PATH )
  find_package(Boost REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})

  # 本体のソースコード
  file( GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp )
  list( REMOVE_ITEM SOURCES ${MAIN_PATH} )

  # ${TEST_NAME}に格納されているファイル名で実行ファイルを作成
  add_executable( ${TEST_NAME} ${TEST_SOURCE_PATH} ${SOURCES} ${TEST_DIRECTORY}/${TEST_MAIN} )
  target_include_directories( ${TEST_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include )

  # コンパイルフラグを追加
  target_compile_options( ${TEST_NAME} PUBLIC
    -Wall
    -target x86_64-w64-windows-gnu)

  # C++標準規格の指定 cxx_std_20はcmake3.12以降で指定可能
  target_compile_features( ${TEST_NAME} PUBLIC cxx_std_20 )

  target_compile_definitions( ${TEST_NAME} PUBLIC BOOST_TEST_NO_LIB=1 )
endfunction( create_executable )
