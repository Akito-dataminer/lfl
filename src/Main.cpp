/****************************************
* Project 
*
* Main.cpp
*****************************************/

#include "version.h"
#include "Util/Comparable.hpp"
#include "jig.hpp"

// std
#include <cstring>
#include <fileapi.h>
#include <iostream>
#include <minwindef.h>
#include <string>
#include <shlwapi.h>
#include <vector>
#include <windows.h>

constexpr char DELIMITER = '\\';
constexpr char OPTION_SPECIFIER[] = "--";
constexpr int SPECIFIER_LENGTH = jig::ArraySize( OPTION_SPECIFIER ); // add the NULL character in the string tail.

namespace message {

// STATIC_CONSTEXPR jig::STRING::Literal VERSION( VERSION_STRING );
STATIC_CONSTEXPR jig::STRING::Literal HELP( "Usage: lfl [directory]\nOutput one name of the latest updated file in specified directory.\ndefault of directory is current directory.\n\nDon't support specification of multiple directories yet.\n" );

template<jig::STRING::Literal MESSAGE, typename CharT, typename Traits>
constexpr void Display( std::basic_ostream<CharT, Traits> & ost ) { ost << MESSAGE; }

} // message

enum class PathTag {
  ARG,
  NON_ARG,
  IMPLICIT,
  NUM
};

bool isDotDirectory( std::string const & directory_path ) {
  // パスの構成要素に'.'かDELIMITER以外が見つかったら、
  // その時点でカレントディレクトリや親ディレクトリそのものを
  // 表したパスではない。
  for ( auto itr : directory_path ) {
    if ( !( ( itr == '.' ) || ( itr == DELIMITER ) ) ) {
      return 0;
    }
  }

  return 1;
}

bool isDirectory( std::string const & path ) {
  // '.'とDELIMITERのみで構成されていたら、
  // その時点でWindows APIを使うまでもなくディレクトリだと分かる。
  if ( isDotDirectory( path ) ) { return 1; }

  return( PathIsDirectory( path.c_str() ) );
}

// Ensure the path exists.
class Path {
public:
  Path() : path_( "" ), exist_( 0 ) {}
  Path( std::string const &, PathTag );
  ~Path();

  Path( const Path & obj ) = default;
  Path & operator = ( const Path & obj ) = delete;
  Path( Path && obj ) noexcept = default;
  Path & operator = ( Path && obj ) noexcept = delete;

  std::string getPath() const noexcept { return path_; }
  PathTag getTag() const noexcept { return tag_; }
  bool isExist() const noexcept { return exist_; }
private:
  std::string path_;
  PathTag tag_;
  // Windows API はなるべく呼び出したくないので、
  // すでに検査したパスはもう一度検査しなくてもいいと示したい。
  const bool exist_;

  bool checkExist() const noexcept;
};

Path::Path( std::string const & path, PathTag tag )
: path_( path ), tag_( tag ), exist_( checkExist() ) {
  // If the path is directory, add path-delimiter into back of argument.
  if ( ( path.back() != DELIMITER ) && isDirectory( path ) ) {
    path_ += DELIMITER;
  }
}

Path::~Path() {
}

bool Path::checkExist() const noexcept { return PathFileExists( path_.c_str() ); }

// enum class OptionKey {
//   DIRECTORY,
//   HELP,
//   NUM
// };

// 基本的にはstd::mapと同じだが、
// ディレクトリが複数指定される可能性もある。
// そのため、キーの重複が許可されている必要があるので、
// std::mapは使えない。
class CmdOption {
public:
  CmdOption( char const *, char const * );
  ~CmdOption();

  constexpr std::string const & getKey() const noexcept { return key_; }
  constexpr std::string const & getValue() const noexcept { return value_; }
  constexpr bool isUnaryOption() const noexcept { return (value_ == "") ? true : false; }
private:
  std::string key_;
  std::string value_;
};

// argはオプションかもしれないし、そうでないかもしれない。
CmdOption::CmdOption( char const * arg, char const * arg_value ) {
  using namespace jig;
  using namespace jig::OPTION;

  STATIC_CONSTEXPR int null_exclude_length = SPECIFIER_LENGTH - 1;

  STATIC_CONSTEXPR char const * options[] = { "help", "version", "directory" };

  if ( std::strlen( arg ) < SPECIFIER_LENGTH ) {
    // argがオプション指定子よりも短い時点で、
    // オプションでないことが確定する。
    key_ = options[jig::ArraySize( options ) - 1];
    value_ = arg;
  } else if ( strncmp( arg, OPTION_SPECIFIER, null_exclude_length ) == 0 ) {
    OptionList<
      STRING::Literal<char const *, STRING::Length(options[0])>( options[0] ),
      STRING::Literal<char const *, STRING::Length(options[1])>( options[1] ),
      STRING::Literal<char const *, STRING::Length(options[2])>( options[2] )
    > option_list;

    char const * key_head = arg + null_exclude_length;
    auto [head_ptr, length] = option_list.isMatch( key_head );
    // std::cerr << "key_head : " << key_head << std::endl;
    // std::cerr << std::string( head_ptr, length ) << std::endl;

    if ( head_ptr != nullptr ) {
      // valueを取るオプションか、valueを取らないオプションかによって、
      // valueに入れる値を変える。
      auto [is_match, index] = option_list.matchIndex( key_head );
      // std::cerr << "std::string( head_ptr ): " << std::string( head_ptr, length ) << std::endl;

      if ( ( index == 0 ) || ( index == 1 ) ) {
        // unary option
        key_ = std::string( head_ptr, length );
        value_ = "";
      } else {
        // binomial option
        key_ = std::string( head_ptr, length );
        value_ = std::string( arg_value, std::strlen( arg_value ) );
      }
    }
  } else {
    key_ = options[jig::ArraySize( options ) - 1];
    value_ = arg;
  }
}

CmdOption::~CmdOption() {
}

class CmdLine {
public:
  CmdLine( int const, char const * [] );
  ~CmdLine();

  CmdOption getOption( int const index ) const noexcept { return options_[index]; }
  int argNum() const noexcept { return argument_num_; }
  std::vector<std::string> optionList( std::string const & );

  bool isThereHelp() const noexcept;
private:
  std::vector<CmdOption> options_;
  int argument_num_;
};

CmdLine::CmdLine( int const arg_count, char const * arg_chars [] )
: argument_num_( 1 ) {
  // コマンドライン上で与えられたすべての文字列を読み込むと前提している。
  // そのため、与えられた文字列が一つだけのときは、
  // 実行パス以外には何も指定されていないということ(=オプションが指定されていない)。
  if ( arg_count != 1 ) {
    try {
      for ( int arg_index = 1; arg_index < arg_count; ++arg_index ) {
        if ( arg_index < ( arg_count - 1 ) ) {
          // std::cerr << "used binomial" << std::endl;
          options_.emplace_back( arg_chars[arg_index], arg_chars[arg_index + 1] );
        } else {
          options_.emplace_back( arg_chars[arg_index], nullptr );
        }

        if ( ( options_.end() - 1 )->isUnaryOption() == false ) { ++arg_index; }
      }
    } catch ( ... ) {
      throw;
    }

    argument_num_ += options_.size();
  }
}

CmdLine::~CmdLine() {
}

std::vector<std::string> CmdLine::optionList( std::string const & key ) {
  std::vector<std::string> option_list;

  for ( auto itr : options_ ) {
    if ( itr.getKey() == key ) { option_list.emplace_back( itr.getValue() ); }
  }

  return option_list;
}

bool CmdLine::isThereHelp() const noexcept {
  // std::cerr << "options_.size(): " << options_.size() << std::endl;

  for ( auto itr : options_ ) {
    // std::cerr << "itr.getKey(): " << itr.getKey() << std::endl;
    if ( itr.getKey() == std::string( "help" ) ) { return true; }
  }

  return false;
}

class Time : public UTIL::COMPARABLE::CompDef<Time> {
public:
  Time() = default;
  Time( FILETIME const & );
  ~Time();

  Time( const Time & obj ) { *this = obj; }
  Time & operator = ( const Time & obj ) {
    if ( this != &obj ) {
      memcpy( &time_, &obj, sizeof( FILETIME ) );
    }
    return *this;
  }

  Time & operator = ( const FILETIME & obj ) {
    memcpy( &time_, &obj, sizeof( FILETIME ) );
    return *this;
  }

  Time( Time && obj ) noexcept = default;
  Time & operator = ( Time && obj ) noexcept = default;

  FILETIME getFileTime() const noexcept { return time_; }

  friend bool operator < ( Time const & time1, Time const & time2 ) noexcept {
    // HighDateTimeに差があればその時点で大小関係が確定する。
    if ( time1.time_.dwHighDateTime != time2.time_.dwHighDateTime ) {
      return( time1.time_.dwHighDateTime < time2.time_.dwHighDateTime );
    } else {
      // HighDateTimeで大小関係が決まらなければ、LowDateTimeで大小関係を決める必要がある。
      return( time1.time_.dwLowDateTime < time2.time_.dwLowDateTime );
    }
  }
private:
  FILETIME time_;
};

Time::Time( FILETIME const & time ) {
  memcpy( &time_, &time, sizeof(FILETIME) );
}

Time::~Time() {
}

void makeCandidate( std::vector<std::string> & candidate_directories, Path const & directory_path ) {
  PathTag tag = directory_path.getTag();
  std::string path_str = directory_path.getPath();

  if ( directory_path.isExist() == true ) {
    if ( ( tag == PathTag::ARG ) || ( tag == PathTag::IMPLICIT ) ) {
      candidate_directories.emplace_back( path_str );
    } else if ( !isDotDirectory( path_str ) ) {
      candidate_directories.emplace_back( path_str );
    }
  }
}

int main( int argc, char const * argv [] ) {
  std::vector<Path *> path_list;

  /* build paths list */
  try {
    CmdLine cmd_line( argc, argv );

    // std::cerr << "cmd_line.argNum(): " << cmd_line.argNum() << std::endl;
    if ( cmd_line.isThereHelp() == true ) {
      using namespace message;
      Display<HELP>( std::cout );
      return 0;
    }

    if ( cmd_line.argNum() == 1 ) {
      path_list.emplace_back( new Path( ".\\", PathTag::IMPLICIT ) );
    } else if ( cmd_line.argNum() > 2 ) {
      std::cout << "yet unimplemented" << std::endl;
      return 0;
    } else {
      std::vector<std::string> directory_list = cmd_line.optionList( "directory" );

      for ( auto itr : directory_list ) {
        path_list.emplace_back( new Path( itr, PathTag::ARG ) );
      }
    }
  } catch ( std::invalid_argument const & e ) {
    using namespace message;
    std::cerr << e.what() << std::endl;

    Display<HELP>( std::cerr );

    for ( auto itr : path_list ) { if ( itr != nullptr ) { delete itr; } }

    return -1;
  } catch ( std::exception const & e ) {
    std::cerr << "error was occured: " << e.what() << std::endl;

    for ( auto itr : path_list ) { if ( itr != nullptr ) { delete itr; } }

    return -1;
  }

  // 存在することが確定したパスだけを判定候補に含める
  std::vector<std::string> exist_directories;
  for ( auto itr : path_list ) {
    if ( isDirectory( itr->getPath() ) ) {
      makeCandidate( exist_directories, *itr );
    } else {
      std::cerr << itr->getPath() << ": is NOT exist" << std::endl;
    }
  }

  // 検査ディレクトリの数が0だったら、これ以上処理を進める必要は無い。
  // コマンドライン引数でディレクトリが指定されているが、
  // そのディレクトリがすべて見つからなかった場合、
  // 検査ディレクトリの数が0になる可能性がある。
  if ( exist_directories.size() == 0 ) {
    std::cout << "There are not any paths to check." << std::endl;

    // for ( auto itr : path_list ) { delete itr; }

    return -2;
  }

  WIN32_FIND_DATA path_data;
  HANDLE hFind;

  /* display the file-name had the latest ftLastWriteTime. */
  Time path_time;
  std::string latest_write_path = "";

  for ( auto itr : exist_directories ) {
    std::string wildcard_path = itr + "*";
    hFind = FindFirstFile( wildcard_path.c_str(), &path_data );

    if ( hFind == INVALID_HANDLE_VALUE ) {
      std::cerr << "INVALID_HANDLE_VALUE" << std::endl;

      FindClose( hFind );
      for ( auto itr : path_list ) { delete itr; }

      return -1;
    }

    /* この時点ですでに、最終更新時刻の検査対象だけのリストが完成しておいてほしい */
    do {
      // ここでほしいのは大小関係だけなので、ローカルタイムに変換したり、
      // SYSTEMTIME構造体に変換したりする必要はない。
      Time current_path_write = path_data.ftLastWriteTime;

      if ( !isDotDirectory( path_data.cFileName ) ) {
        // debug
        // std::cout << path_data.ftLastWriteTime.dwHighDateTime << ", " << path_data.ftLastWriteTime.dwLowDateTime << " : " << path_data.cFileName << std::endl;
        // std::cout << path_time.getFileTime().dwHighDateTime << ", " << path_time.getFileTime().dwLowDateTime << " : " << latest_write_path << std::endl;

        if ( path_time < current_path_write ) { path_time = current_path_write; latest_write_path = path_data.cFileName; }
      }
    } while ( FindNextFile( hFind, &path_data ) != 0 );

    FindClose( hFind );
  }

  for ( auto itr : path_list ) { delete itr; }

  /* display the latest updated file name */
  std::cout << latest_write_path << std::endl;

  return 0;
}
