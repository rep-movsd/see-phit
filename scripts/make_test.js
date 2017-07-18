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
  
  SPTNode root = SPTNode::from(parser.nodes);
  root.dump(cerr);
  
}

`;

console.log(src);



