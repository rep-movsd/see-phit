#pragma once 

// Allow runtime debugging for development
#ifdef SPT_DEBUG

#define constexpr 
#define DUMP cerr
#define ENDL "\n"

// Dummy function to prettify compile-time errors
void ParseError(const char* err) {cerr << "Parse Error:" << err << endl;}
#define PARSE_ERR(x) ParseError(x)

#else

#define DUMP DummyOut
#define ENDL 0

struct DummyOutStream
{
  template<typename T>  constexpr const DummyOutStream& operator <<(const T &) const { return *this;}
};

constexpr DummyOutStream DummyOut;

template<int SIZE>
struct ErrLine
{
  char dummy[SIZE];
  constexpr char &operator[](size_t i) { return dummy[i];};
};

// Dummy function to prettify compile-time errors
constexpr ErrLine<0> ParseError(const char*)
{
  return ErrLine<0>{};
}

// Dummy function to prettify compile-time errors
constexpr ErrLine<1> ParseWarning(const char*)
{
  return ErrLine<1>{};
}

#define PARSE_ERR(x) \
int iLine = cur_line(); \
ParseError(x)[iLine] 

#define PARSE_WARN(x) \
ParseWarning(x)[0] = 256;


#endif
