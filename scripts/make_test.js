const file = process.argv[2];
const src = 
`
#include <iostream>
#include <type_traits>
#include <chrono>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto parser = 
  #include "${file}"
  
  REPORT_ERRORS(parser);
  
  spt::tree spt_tree(parser);
  spt::template_vals dct;
  spt::template_funs dctFuns;
    
  spt_tree.root().render(cout, dct, dctFuns);
  cout << endl;
}
`;

console.log(src);



