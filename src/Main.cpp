/****************************************
* Project 
*
* Main.cpp
*****************************************/

#include "Util/Comparable.hpp"

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
  if ( ( path.back() != DELIMITER ) && isDirectory(path) ) {
    path_ += DELIMITER;
  }
}

Path::~Path() {
}

bool Path::checkExist() const noexcept { return PathFileExists( path_.c_str() ); }

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

int main( int argc, char * argv [] ) {
  std::vector<Path *> path_list;

  /* build paths list */
  try {
    if ( argc == 1 ) {
      path_list.emplace_back( new Path( ".\\", PathTag::IMPLICIT ) );
    } else if ( argc > 3 ) {
      std::cout << "yet unimplemented" << std::endl;
      return 0;
    } else {
      for ( int arg_index = 0; arg_index != argc; ++arg_index ) {
        path_list.emplace_back( new Path( argv[arg_index], PathTag::ARG ) );
      }
    }
  } catch ( ... ) {
    std::cerr << "error was occured" << std::endl;
    for ( auto itr : path_list ) { if ( itr != nullptr ) { delete itr; } }
    return -1;
  }

  // 存在することが確定したパスだけを判定候補に含める
  std::vector<std::string> exist_directories;
  for ( auto itr : path_list ) {
    if ( isDirectory( itr->getPath() ) ) {
      makeCandidate( exist_directories, *itr );
    }
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
        std::cout << path_data.ftLastWriteTime.dwHighDateTime << ", " << path_data.ftLastWriteTime.dwLowDateTime << " : " << path_data.cFileName << std::endl;
        std::cout << path_time.getFileTime().dwHighDateTime << ", " << path_time.getFileTime().dwLowDateTime << " : " << latest_write_path << std::endl;

        if ( path_time < current_path_write ) { path_time = current_path_write; latest_write_path = path_data.cFileName; }
      }
    } while ( FindNextFile( hFind, &path_data ) != 0 );

    FindClose( hFind );
  }

  /* display the latest updated file name */
  std::cout << std::endl;
  std::cout << path_time.getFileTime().dwHighDateTime << ", " << path_time.getFileTime().dwLowDateTime << " : " << latest_write_path << std::endl;

  for ( auto itr : path_list ) { delete itr; }
  return 0;
}
