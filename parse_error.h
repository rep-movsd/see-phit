#ifndef SEEPHIT_PARSE_ERROR_H
#define SEEPHIT_PARSE_ERROR_H

#include "pch.h"

namespace spt
{

#include "parse_error_generated.h"  

struct Message
{
  enum Messages m = Error_None;
  int row = -1;
  int col = -1;
  constexpr Message(): row(), col() {}
  constexpr Message(Messages m, int row, int col):m(m), row(row), col(col) {}
};


template<int ROW, int COL, typename WHAT> struct Warning 
{
  char SPTParser;
  Warning() 
  { 
    if(ROW != -1 || COL != -1)    
    {
      constexpr char x[0] = {};
      SPTParser = x[1];
      
      const int warning = 256;
      SPTParser = warning;
    }
  }  
};

template<int ROW, int COL, typename WHAT> struct Error 
{
  char SPTParser[0] = {};
  constexpr Error() 
  { 
    const int fatal = 0;
    SPTParser = fatal;
  }  
};


template<bool Cond, class T = void> struct IF;
template<class T> struct IF<true, T> { T it; };
template<class T> struct IF<false, T> {};

}  // namespace spt
#endif
