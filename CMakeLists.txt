# cmakeの最小バージョンを指定
cmake_minimum_required( VERSION 3.22.1 )

# プロジェクト名と使用する言語を指定
project( CMakeTest CXX )

# compile_commands.jsonを出力するように設定
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

# コンパイラを指定
set( CMAKE_CXX_COMPILER clang++ )

# CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDESがONになっていないと
# unixスタイルのインクルードパス指定になるため、
# clangdがインクルードパスを読み込んでくれない。
#
# しかし、CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDESをONにして、
# インクルードパスを別ファイルに出力するようにしていると、
# その別ファイルにはWindowsスタイルでパスが書き込まれるため、
# その問題をクリアできる。
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES ON)

# 利用できるプロセッサ数を調べる
include(ProcessorCount)
ProcessorCount(PROCESSOR_NUM)

# ProcessorCountが失敗していないことを確かめる
if(NOT PROCESSOR_NUM EQUAL 0)
  set(CTEST_BUILD_FLAGS -j${PROCESSOR_NUM})
  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${PROCESSOR_NUM})
endif()

# サブディレクトリを追加
add_subdirectory( src )

# Enable the testing features
enable_testing()
add_subdirectory( test )
