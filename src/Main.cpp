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
#include <windows.h>

// constexpr char DELIMITER = '\\';

// Ensure the path exists.
class Path {
public:
  Path() : path_( "" ), exist_( 0 ) {}
  Path( std::string const & );
  ~Path();

  Path( const Path & obj ) = default;
  Path & operator = ( const Path & obj ) = delete;
  Path( Path && obj ) noexcept = default;
  Path & operator = ( Path && obj ) noexcept = delete;

  std::string getPath() const noexcept { return path_; }
private:
  std::string path_;
  // Windows API はなるべく呼び出したくないので、
  // すでに検査したパスはもう一度検査しなくてもいいと示したい。
  const bool exist_;

  bool isExist() const noexcept;
};

Path::Path( std::string const & path )
: path_( path ), exist_( isExist() ) {
}

Path::~Path() {
}

bool Path::isExist() const noexcept {
  return PathFileExists( path_.c_str() );
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

int main( int argc, char * argv [] )
{
  Path path( ".\\" );

  if ( argc > 1 ) {
    std::cerr << "Argument specification is NOT yet implemented" << std::endl;
    return 0;
  }

  WIN32_FIND_DATA path_data;

  std::string wildcard_path = path.getPath() + "*";
  HANDLE hFind = FindFirstFile( wildcard_path.c_str(), &path_data );

  if ( hFind == INVALID_HANDLE_VALUE ) {
    std::cerr << "INVALID_HANDLE_VALUE" << std::endl;
    FindClose( hFind );
    return -1;
  }

  /* display the file-name had the latest ftLastWriteTime. */
  Time path_time;
  std::string latest_write_path = "";

  do {
    // この階層や一つ上の階層は必要ないのでスキップ
    if ( path_data.cFileName[0] == '.' ) {
      if ( path_data.cFileName[1] == '\0'
        || ( path_data.cFileName[1] == '.' && path_data.cFileName[2] == '\0' ) ) {
        if ( FindNextFile( hFind, &path_data ) != 0 ) {
          continue;
        } else {
          break;
        }
      }
    }

    // ここでほしいのは大小関係だけなので、ローカルタイムに変換したり、
    // SYSTEMTIME構造体に変換したりする必要はない。
    Time current_path_write = path_data.ftLastWriteTime;

    // debug
    // std::cout << path_data.ftLastWriteTime.dwHighDateTime << ", " << path_data.ftLastWriteTime.dwLowDateTime << " : " << path_data.cFileName << std::endl;
    // std::cout << path_time.getFileTime().dwHighDateTime << ", " << path_time.getFileTime().dwLowDateTime << " : " << latest_write_path << std::endl;

    if ( path_time < current_path_write ) { path_time = current_path_write; latest_write_path = path_data.cFileName; }
  } while ( FindNextFile( hFind, &path_data ) != 0 );

  FindClose( hFind );

  /* display the latest updated file name */
  std::cout << path_time.getFileTime().dwHighDateTime << ", " << path_time.getFileTime().dwLowDateTime << " : " << latest_write_path << std::endl;

  return 0;
}
