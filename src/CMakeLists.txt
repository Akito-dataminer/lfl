file( GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp )

# 生成するファイル名の指定
set( OUTPUT lfl )

# 実行ファイル生成のターゲットを生成する
set( INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include ) # インクルードディレクトリの設定
set( VERSION_INFORMATION_HEADER ${INCLUDE_DIRECTORY}/version.h ) # バージョン情報ヘッダを出力するための設定

# ここでVERSION_INFORMATION_HEADERを指定しておかないと、ビルド時に生成されない
add_executable( ${OUTPUT} ${SOURCES} ${VERSION_INFORMATION_HEADER} )
target_include_directories( ${OUTPUT} PUBLIC ${INCLUDE_DIRECTORY} )

# コンパイルフラグを追加
target_compile_options(${OUTPUT} PUBLIC
  -Wall
  -target x86_64-w64-windows-gnu)

# C++標準規格の指定 cxx_std_20はcmake3.12以降で指定可能
target_compile_features( ${OUTPUT} PUBLIC cxx_std_20 )

##############################
# バージョン情報を定義したヘッダファイルを生成する
##############################
set( VERSION_SCRIPT_PATH ${PROJECT_SOURCE_DIR}/version.cmake )
set( VERSION_CONFIGURE_PATH ${PROJECT_SOURCE_DIR}/version.h.in )

add_custom_command(OUTPUT ${VERSION_INFORMATION_HEADER}
  DEPENDS ${VERSION_SCRIPT_PATH} ${VERSION_CONFIGURE_PATH}
  COMMAND ${CMAKE_COMMAND} -D TOOL_NAME="${CMAKE_PROJECT_NAME}" -P ${VERSION_SCRIPT_PATH}
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# リンクするライブラリの指定
find_library(SHLWAPI_LIBRARY shlwapi)
target_link_libraries(${OUTPUT} PUBLIC ${SHLWAPI_LIBRARY})
