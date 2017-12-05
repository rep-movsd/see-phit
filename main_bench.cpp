#include <iostream>
#include <type_traits>
#include <chrono>
#include "seephit.h"
using namespace std;



int main()
{
  constexpr auto parser = 
  #include "test/loop_bench.spt"
    
  REPORT_ERRORS(parser);
    
  // Start timer and run it
  auto tmStart = chrono::high_resolution_clock::now();
  
  int k;
  for(int i = 0; i < 1; ++i)
  {
    spt::tree spt_tree(parser);
    spt::template_vals dct;
    spt::template_funs dctFuns;
    
    dctFuns["double"] = 
    [](ostream &ostr, const string &sKey, spt::template_vals &vals)
    {
      ostr << std::get<int>(vals[sKey]) * 2;
    };
    
    dctFuns["quote"] = 
    [](ostream &ostr, const string &sKey, spt::template_vals &vals)
    {
      ostr << '\'' << std::get<int>(vals[sKey]) << '\'';
    };
    
    
    spt_tree.root().render(cout, dct, dctFuns);
    k = dct.size();
  }
  
  auto tmElapsed = chrono::high_resolution_clock::now() - tmStart;
  long long nano = chrono::duration_cast<std::chrono::nanoseconds>(tmElapsed).count();
  double ms = nano/1000000.0F;
  cerr << ms << " ms elapsed" << endl;
  cerr << k << " unique template keys" << endl;
  
  cerr << endl;
}


