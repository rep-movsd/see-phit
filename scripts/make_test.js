const file = process.argv[2];
const src = 
`
#include <iostream>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto parser =
    #include "${file}" 
  
  spt::tree spt_tree(parser);
  spt::template_dict dct;
  spt_tree.root.dump(cout, dct);
}
`;

console.log(src);



