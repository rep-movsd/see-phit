#ifndef SEEPHIT_DEBUG_H
#define SEEPHIT_DEBUG_H

#pragma once 

// Allow runtime debugging for development
#ifdef SPT_DEBUG

#define constexpr 
#define DUMP cerr
#define ENDL "\n"

// Dummy function to prettify compile-time errors
#define PARSE_ERR(x) {cerr << "Parse Error:" << #x << endl;}
#define PARSE_WARN(x) {cerr << "Parse Warning:" << #x << endl;}


#else

#define DUMP DummyOut
#define ENDL 0

struct DummyOutStream
{
  template<typename T>  constexpr const DummyOutStream& operator <<(const T & /*unused*/) const { return *this;}
};

constexpr DummyOutStream DummyOut;

struct ErrLine
{
  int dummy[0];
  constexpr int &operator[](size_t i) { return dummy[i];};
};

// Dummy function to prettify compile-time errors
constexpr ErrLine ParseError(const char* /*unused*/)
{
  return ErrLine{};
}

// Set error message and location if not already set
#define PARSE_ERR(x) if(m_iErrRow == -1) {m_iErrRow = cur_row(); m_iErrCol = cur_col(); m_arrErrs = x;}

// Push warning message and location to list
#define PARSE_WARN(x) m_arrWarns.push_back(Message(x, cur_row(), cur_col()))


#endif

#endif
