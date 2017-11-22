#include <iostream>
#include <type_traits>
#include <chrono>
#include "seephit.h"
using namespace std;



int main()
{
  constexpr auto parser = 
  #include "test/loop_bad.spt"
    
  REPORT_ERRORS(parser);
    
  spt::tree spt_tree(parser);
  spt::template_dict dct = spt_tree.get_default_dict();
  spt_tree.root().render(cout, dct);
  
  cout << endl;
  
  //parser.dump();
  cout << endl;
  
}


