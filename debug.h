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
  template<typename T>  constexpr const DummyOutStream& operator <<(const T &) const { return *this;}
};

constexpr DummyOutStream DummyOut;

struct ErrLine
{
  int dummy[0];
  constexpr int &operator[](size_t i) { return dummy[i];};
};

// Dummy function to prettify compile-time errors
constexpr ErrLine ParseError(const char*)
{
  return ErrLine{};
}

#define PARSE_ERR(x) errRow = cur_row(); errCol = cur_col(); err = x
#define PARSE_WARN(x) warns.push_back(Message(x, cur_row(), cur_col()))



#endif
