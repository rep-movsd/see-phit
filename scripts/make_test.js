const file = process.argv[2];
const src = 
`
#include <iostream>
#include "seephit.h"
using namespace std;

int main()
{
  constexpr auto nodes = 
  #include "${file}" 
  
  dumpNode(nodes, 0, 0);
}

`;

console.log(src);



