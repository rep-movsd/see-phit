#include <iostream>
#include <type_traits>
#include <chrono>
#include "seephit.h"
using namespace std;


int main()
{
  constexpr auto parser = 
  #include "test/loop.spt"
    
  REPORT_ERRORS(parser);
    
  spt::tree spt_tree(parser);
  spt::template_vals dct;
  spt::template_funs dctFuns;
  
  dct["s"] = "this should be quoted";
  
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
  cout << endl;
  
  //parser.dump();
  cout << endl;
  
}


