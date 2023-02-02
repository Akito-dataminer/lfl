/****************************************
* Project 
*
* Main.cpp
*****************************************/

#include "version.h"
#include "Util/Comparable.hpp"
#include "jig/option.hpp"

// std
#include <cstring>
#include <fileapi.h>
#include <iostream>
#include <minwindef.h>
#include <string>
#include <shlwapi.h>
#include <vector>
#include <windows.h>

STATIC_CONSTEXPR char DELIMITER = '\\';
STATIC_CONSTEXPR char OPTION_SPECIFIER[] = "--";
STATIC_CONSTEXPR int SPECIFIER_LENGTH = jig::ArraySize( OPTION_SPECIFIER ); // add the NULL character in the string tail.
STATIC_CONSTEXPR int NULL_EXCLUDE_OS_LENGTH = SPECIFIER_LENGTH - 1;
STATIC_CONSTEXPR char const * options[] = { "help", "version", "directory" };
STATIC_CONSTEXPR jig::OPTION::OptionList<
jig::STRING::Literal<char const *, jig::STRING::Length(options[0])>( options[0] ),
jig::STRING::Literal<char const *, jig::STRING::Length(options[1])>( options[1] ),
jig::STRING::Literal<char const *, jig::STRING::Length(options[2])>( options[2] )
> option_list;

namespace message {

STATIC_CONSTEXPR jig::STRING::Literal VERSION( VERSION_STRING );
STATIC_CONSTEXPR jig::STRING::Literal HELP( "Usage: lfl [directory]\nOutput one name of the latest updated file in specified directory.  default of directory is current directory.\nDon't support specification of multiple directories yet." );
STATIC_CONSTEXPR jig::STRING::Literal USAGE_DIRECTORY( "--directory: Specify search directories.\n  ( This option is always specified if none is specified )." );
STATIC_CONSTEXPR jig::STRING::Literal USAGE_HELP( "--help: Display this message." );
STATIC_CONSTEXPR jig::STRING::Literal USAGE_VERSION( "--version: Display the version of this application." );

STATIC_CONSTEXPR jig::STRING::Literal NEW_LINE( "\n" );
STATIC_CONSTEXPR jig::STRING::Literal DOUBLE_NEW( "\n\n" );

STATIC_CONSTEXPR jig::STRING::Literal HELP_MESSAGE = HELP + DOUBLE_NEW + USAGE_DIRECTORY + NEW_LINE + USAGE_HELP + NEW_LINE + USAGE_VERSION + NEW_LINE;

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

class CmdArg {
public:
  constexpr explicit CmdArg( char const *, bool const );
  constexpr CmdArg( CmdArg const & ) = default;
  constexpr CmdArg & operator=( CmdArg const & ) = default;
  constexpr CmdArg( CmdArg && ) noexcept = default;
  constexpr CmdArg & operator=( CmdArg && ) noexcept = default;
  constexpr ~CmdArg();

  constexpr inline std::string const & str() const noexcept { return arg_; }
  constexpr inline bool isOption() const noexcept { return is_option_; }
  constexpr inline bool isBinomial() const noexcept { return is_binomial_; }
private:
  std::string arg_;
  bool is_option_;
  bool is_binomial_;
};

constexpr CmdArg::CmdArg( char const * arg, bool const has_option_specifier )
: arg_( arg ), is_option_( false ), is_binomial_( false ) {
  if ( has_option_specifier ) {
    auto [ is_option, option_index ] = option_list.matchIndex( arg );
    is_option_ = is_option;

    // 現状では、二項オプションかどうかは、
    // option_listの添字で判断するしかない。
    if ( is_option ) {
      if ( !( option_index == 0 || option_index == 1 ) ) { is_binomial_ = true; }
    }
  }
}

constexpr CmdArg::~CmdArg() {}

class CmdParse {
public:
  using value_type = CmdArg;
  using container_type = std::vector<value_type>;

  constexpr CmdParse( int const, char const *[] );
  constexpr CmdParse( CmdParse const & ) = default;
  constexpr CmdParse & operator=( CmdParse const & rhs ) = default;
  constexpr CmdParse( CmdParse && ) = default;
  constexpr CmdParse & operator=( CmdParse && ) = default;
  constexpr ~CmdParse();

  constexpr std::pair<std::string, std::string> get();

  constexpr inline bool isThereHelp() const noexcept { return is_there_help_; }
  constexpr inline bool next() noexcept {
    // std::cerr << "index_ : " << index_ << std::endl;
    index_ += ( arg_list_[index_].isBinomial() ) ? 2 : 1;
    // std::cerr << "boolean : " << ( index_ >= arg_list_.size() ) << std::endl;
    return ( index_ >= arg_list_.size() );
  }

  constexpr inline bool isEnd() const noexcept { return ( index_ == arg_list_.size() - 1 ); }
private:
  std::size_t index_;
  container_type arg_list_;
  bool is_there_help_;

  constexpr inline std::size_t lengthAsOption( char const * str ) const noexcept { auto [ ptr, length ] = option_list.isMatch( str ); return ptr != nullptr ? length : 0; }
};

// 何個目が二項オプションか、単項オプションか、あるいは、それはオプションかどうか
// といったような、指定されたオプションの構造を調べる。
constexpr CmdParse::CmdParse( int const arg_count, char const * arg_chars[] )
:index_( 0 ), is_there_help_( false ) {
  // オプションの構造解析フェーズ
  for ( std::size_t arg_index = 1; arg_index < arg_count; ++arg_index ) {
    // この時点ではまだ、arg_chars[arg_index]に"--"が付けられているのか、
    // 付けられていないのかが分からない。
    if ( !std::strncmp( arg_chars[arg_index], OPTION_SPECIFIER, NULL_EXCLUDE_OS_LENGTH ) ) {
      arg_list_.emplace_back( CmdArg( arg_chars[arg_index] + NULL_EXCLUDE_OS_LENGTH, true ) );
    } else {
      arg_list_.emplace_back( CmdArg( arg_chars[arg_index], false ) );
    }
  }

  // --helpがコマンドライン引数に含まれているかどうかを調べる。
  for ( auto itr : arg_list_ ) {
    if ( itr.isOption() ) {
      if ( itr.str() == "help" ) {
        is_there_help_ = true;
        break;
      }
    }
  }
}

constexpr CmdParse::~CmdParse() {}

constexpr std::pair<std::string, std::string> CmdParse::get() {
  if ( index_ == arg_list_.size() ) { return std::pair( "", "" ); }

  if ( is_there_help_ ) { return std::pair( "help", arg_list_[index_].str() ); }

  if ( arg_list_[index_].isOption() ) {
    if ( arg_list_[index_].isBinomial() ) {
      return std::pair( arg_list_[index_].str(), arg_list_[index_ + 1].str() );
    } else {
      return std::pair( arg_list_[index_].str(), "" );
    }
  }

  return std::pair( "directory", arg_list_[index_].str() );
}

// 基本的にはstd::mapと同じだが、
// ディレクトリが複数指定される可能性もある。
// そのため、キーの重複が許可されている必要があるので、
// std::mapは使えない。
class CmdOption {
public:
  constexpr CmdOption( std::string const &, std::string const & );
  constexpr CmdOption( std::pair<std::string, std::string> const & );
  constexpr ~CmdOption();

  constexpr std::string const & getKey() const noexcept { return key_; }
  constexpr std::string const & getValue() const noexcept { return value_; }
private:
  std::string key_;
  std::string value_;
};

constexpr CmdOption::CmdOption( std::string const & key, std::string const & value ) : key_( key ), value_( value ) {}

constexpr CmdOption::CmdOption( std::pair<std::string, std::string> const & pair ) : key_( pair.first ), value_( pair.second ) {}

constexpr CmdOption::~CmdOption() {}

class CmdLine {
public:
  constexpr CmdLine( int const, char const * [] );
  constexpr ~CmdLine();

  constexpr CmdOption const & getOption( int const index ) const noexcept { return options_[index]; }
  constexpr int argNum() const noexcept { return argument_num_; }
  constexpr std::vector<std::string> optionList( std::string const & );

  template<std::size_t N>
  constexpr bool isThere( char const (&)[N] ) const noexcept;
private:
  std::vector<CmdOption> options_;
  int argument_num_;
};

constexpr CmdLine::CmdLine( int const arg_count, char const * arg_chars [] )
: argument_num_( 1 ) {
  try {
    // コマンドライン上で与えられたすべての文字列を読み込むと前提している。
    // そのため、与えられた文字列が一つだけのときは、
    // 実行パス以外には何も指定されていないということ(=オプションが指定されていない)。
    if ( arg_count != 1 ) {
      CmdParse parse( arg_count, arg_chars );

      do { options_.emplace_back( CmdOption( parse.get() ) ); } while( !parse.next() );

      argument_num_ = options_.size();
    } else {
      // for ( std::size_t arg_index = 1; arg_index < arg_count; ++arg_index ) {
      //   options_.emplace_back( arg_chars[arg_index] );
      // }
    }
  } catch ( ... ) {
    throw;
  }
}

constexpr CmdLine::~CmdLine() {}

constexpr std::vector<std::string> CmdLine::optionList( std::string const & key ) {
  std::vector<std::string> option_list;

  for ( auto itr : options_ ) {
    if ( itr.getKey() == key ) { option_list.emplace_back( itr.getValue() ); }
  }

  return option_list;
}

template<std::size_t N>
constexpr bool CmdLine::isThere( char const ( & option )[N] ) const noexcept {
  for ( auto itr : options_ ) {
    if ( std::strncmp( itr.getKey().c_str(), option, N ) == 0 ) { return true; }
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

    if ( cmd_line.isThere( "help" ) ) {
      using namespace message;

      auto help_list = cmd_line.optionList( "help" );

      if ( help_list.size() == 0 ) {
        Display<HELP_MESSAGE>( std::cout );
        return 0;
      } else {
        for ( auto itr : help_list ) {
          if ( itr == "help" ) { Display<USAGE_HELP>( std::cout ); }
          if ( itr == "version" ) { Display<USAGE_VERSION>( std::cout ); }
          if ( itr == "directory" ) { Display<USAGE_DIRECTORY>( std::cout ); }
        }
      }
    }

    if ( cmd_line.isThere( "version" ) ) {
      using namespace message;
      Display<VERSION>( std::cout );
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

    Display<HELP_MESSAGE>( std::cerr );

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
